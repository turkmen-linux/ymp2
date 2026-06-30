#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <core/ymp.h>
#include <utils/error.h>
#include <core/logger.h>

#include <utils/array.h>
#include <utils/string.h>

typedef struct {
    const char* name;
    size_t line;
} label;

static label *labels;
size_t label_max = 32;
size_t label_cur = 0;

static size_t* while_stack = NULL;
static size_t while_stack_max = 32;
static size_t while_depth = 0;

static size_t* ret_stack = NULL;
static size_t ret_stack_max = 32;
static size_t ret_depth = 0;

static void labels_unref() {
    if (!labels) {
        return;
    }
    for (size_t i=0; i < label_cur; i++) {
        free((char*)labels[i].name);
    }
    free(labels);
    label_cur = 0;
}

static char* exec_capture(const char* cmd) {
    char buf[4096];
    FILE* fp = popen(cmd, "r");
    if (!fp) return strdup("");
    size_t n = fread(buf, 1, sizeof(buf) - 1, fp);
    pclose(fp);
    buf[n] = '\0';
    while (n > 0 && (buf[n-1] == '\n' || buf[n-1] == '\r')) buf[--n] = '\0';
    return strdup(buf);
}

static char** tokenize_line(const char* line, bool* stop_expr) {
    size_t cap = 32, cnt = 0;
    char** args = calloc(cap, sizeof(char*));
    char tok[4096];
    size_t tpos = 0;
    bool dq = false, sq = false, literal = false;
    if (stop_expr) *stop_expr = false;

    while (*line) {
        char c = *line;

        if (!dq && !sq && (c == ' ' || c == '\t')) {
            line++;
            if (tpos > 0) {
                tok[tpos] = '\0';
                args[cnt] = strdup(tok);
                cnt++;
                tpos = 0;
                if (cnt >= cap - 1) {
                    cap += 32;
                    args = realloc(args, cap * sizeof(char*));
                }
            }
            continue;
        }

        if (c == '\'' && !dq) { sq = !sq; line++; continue; }
        if (c == '"' && !sq) { dq = !dq; line++; continue; }

        if (sq) { tok[tpos++] = c; line++; continue; }
        if (literal) { tok[tpos++] = c; line++; continue; }

        if (c == '\\') {
            line++;
            if (*line) { tok[tpos++] = *line; line++; }
            continue;
        }

        if (c == '$') {
            line++;
            if (*line == '(') {
                line++;
                size_t depth = 1;
                const char* start = line;
                while (*line && depth > 0) {
                    if (*line == '(') depth++;
                    else if (*line == ')') depth--;
                    if (depth > 0) line++;
                }
                size_t clen = line - start;
                char* cmd = malloc(clen + 1);
                memcpy(cmd, start, clen); cmd[clen] = '\0';
                if (*line == ')') line++;
                char* out = exec_capture(cmd);
                size_t olen = strlen(out);
                if (tpos + olen < sizeof(tok) - 1) {
                    memcpy(tok + tpos, out, olen);
                    tpos += olen;
                }
                free(out); free(cmd);
                continue;
            } else if (*line == '{') {
                line++;
                const char* start = line;
                while (*line && *line != '}') line++;
                size_t vlen = line - start;
                char* vname = malloc(vlen + 1);
                memcpy(vname, start, vlen); vname[vlen] = '\0';
                const char* val = get_value(vname);
                if (val) {
                    size_t vallen = strlen(val);
                    if (tpos + vallen < sizeof(tok) - 1) {
                        memcpy(tok + tpos, val, vallen);
                        tpos += vallen;
                    }
                }
                free(vname);
                if (*line == '}') line++;
                continue;
            } else {
                const char* start = line;
                while (*line && ((*line >= 'a' && *line <= 'z') || (*line >= 'A' && *line <= 'Z') || (*line >= '0' && *line <= '9') || *line == '_')) line++;
                if (line > start) {
                    size_t vlen = line - start;
                    char* vname = malloc(vlen + 1);
                    memcpy(vname, start, vlen); vname[vlen] = '\0';
                    const char* val = get_value(vname);
                    if (val) {
                        size_t vallen = strlen(val);
                        if (tpos + vallen < sizeof(tok) - 1) {
                            memcpy(tok + tpos, val, vallen);
                            tpos += vallen;
                        }
                    }
                    free(vname);
                }
                continue;
            }
        }

        if (c == '`') {
            line++;
            const char* start = line;
            while (*line && *line != '`') line++;
            size_t clen = line - start;
            char* cmd = malloc(clen + 1);
            memcpy(cmd, start, clen); cmd[clen] = '\0';
            if (*line == '`') line++;
            char* out = exec_capture(cmd);
            size_t olen = strlen(out);
            if (tpos + olen < sizeof(tok) - 1) {
                memcpy(tok + tpos, out, olen);
                tpos += olen;
            }
            free(out); free(cmd);
            continue;
        }

        if (c == '-' && tpos == 0 && line[1] == '-') {
            line += 2;
            if (*line == ' ' || *line == '\t' || *line == '\0') {
                if (stop_expr) *stop_expr = true;
                literal = true;
                continue;
            }
            tok[tpos++] = '-';
            tok[tpos++] = '-';
            line++;
            continue;
        }

        tok[tpos++] = c;
        line++;
    }

    if (tpos > 0) {
        tok[tpos] = '\0';
        args[cnt] = strdup(tok);
        cnt++;
    }
    args[cnt] = NULL;
    return args;
}

visible char** parse_args(char** args, bool free_strings) {

    size_t len = 0;
    size_t offset = 0;

    for(len=0; args[len];len++){}

    for(size_t i=0; i<len;i++){
        debug("parse: %s\n", args[i]);
        if (strlen(args[i]) > 2 && args[i][0] == '-' && args[i][1] == '-'){
            for(offset=0; args[i][offset] && args[i][offset]!='='; offset++){}
            char var[offset-1];
            strncpy(var, args[i]+2, offset-2);
            var[offset-2]= '\0';
            debug("offset=%lld len=%lld\n", offset, strlen(args[i]));
            if (offset >= strlen(args[i])){
                set_bool(var, true);
            } else {
                char val[strlen(args[i]) - offset - 1];
                strncpy(val, args[i]+offset+1, strlen(args[i]) - offset - 1);
                val[strlen(args[i]) - offset -1] = '\0';
                set_value(var, val);
            }
            if (free_strings) {
                free(args[i]);
            }
            args[i]=NULL;
        }
    }
    size_t removed = 0;
    for(size_t i=0; i<len;i++){
        if (args[i] == NULL){
            removed++;
            continue;
        }
        args[i-removed] = args[i];
    }
    for (size_t i=len-removed;args[i]; i++){
        args[i] = NULL;
    }
    return args;
}

static char** parse_line(const char* line){
    bool stop_expr = false;
    char** ret = tokenize_line(line, &stop_expr);
    return parse_args(ret, true);
}

static bool is_cond_op(const char* s) {
    return s && (iseq(s, "and") || iseq(s, "or") || iseq(s, "not"));
}

static int eval_cond_chain(char** args, size_t* pos) {
    if (!args[*pos]) return 1;

    if (iseq(args[*pos], "not")) {
        (*pos)++;
        return !eval_cond_chain(args, pos);
    }

    char* op_name = args[*pos];
    (*pos)++;

    size_t start = *pos;
    size_t count = 0;
    while (args[start + count] && !is_cond_op(args[start + count])) count++;

    char** op_args = malloc((count + 1) * sizeof(char*));
    for (size_t i = 0; i < count; i++) op_args[i] = args[start + i];
    op_args[count] = NULL;

    int result = 1;
    Operation op = get_operation_by_name(global->manager, op_name);
    if (op.call && count >= op.min_args)
        result = op.call(op_args);

    free(op_args);
    *pos = start + count;
    return result;
}

static bool eval_conditions(char** args) {
    size_t pos = 0;
    int result = eval_cond_chain(args, &pos);
    while (args[pos]) {
        if (iseq(args[pos], "and")) {
            pos++;
            int right = eval_cond_chain(args, &pos);
            result = (result == 0 && right == 0) ? 0 : 1;
        } else if (iseq(args[pos], "or")) {
            pos++;
            int right = eval_cond_chain(args, &pos);
            result = (result == 0 || right == 0) ? 0 : 1;
        } else {
            break;
        }
    }
    return result == 0;
}

visible int run_script(const char* script){
    char** lines = split(script, "\n");
    int rc = 0;
    int iflevel = 0;
    if(!labels) {
        labels = calloc(label_max, sizeof(label));
    }
    if(!while_stack) {
        while_stack = calloc(while_stack_max, sizeof(size_t));
    }
    if(!ret_stack) {
        ret_stack = calloc(ret_stack_max, sizeof(size_t));
    }
    for(size_t i=0; lines[i]; i++){
        char* ltmp = strip(lines[i]);
        free(lines[i]);
        lines[i] = ltmp;
        if(startswith(lines[i], "label ")){
            if(label_cur >= label_max){
                label_max+= 32;
                label* tmp = realloc(labels, sizeof(label)* label_max);
                if(tmp) labels = tmp;
            }
            labels[label_cur].name = strdup(lines[i]+6);
            labels[label_cur].line = i;
            label_cur++;
        }
    }
    for(size_t i=0; lines[i]; i++){
        if(strlen(lines[i]) == 0) continue;
        if(lines[i][0] == '#') continue;
        debug("script:%s\n", lines[i]);
        char** args = parse_line(lines[i]);
        if (!args[0]) { free(args); continue; }

        if (iseq(args[0], "label")) {
            for (i++; lines[i]; i++) {
                char* t = strip(lines[i]);
                if (strlen(t) == 0 || t[0] == '#') { free(t); continue; }
                free(t);
                char** la = parse_line(lines[i]);
                if (la && iseq(la[0], "ret")) {
                    for(size_t j=0; la[j]; j++) free(la[j]);
                    free(la);
                    break;
                }
                for(size_t j=0; la[j]; j++) free(la[j]);
                free(la);
            }
            goto args_cleanup;
        }
        if (iseq(args[0], "if")){
            if (!eval_conditions(args + 1)) {
                iflevel++;
                int cur = i;
                for(i=i+1;lines[i]; i++){
                    if(strlen(lines[i]) == 0) continue;
                    if(startswith(lines[i], "endif")) iflevel--;
                    if(startswith(lines[i], "if ")) iflevel++;
                    if(iflevel == 0) goto found_endif;
                }
                error_add(build_string("syntax error at line %d : endif missing", cur));
                error(1);
                goto args_cleanup_err;
                found_endif:;
            }
            goto args_cleanup;
        }
        if (iseq(args[0], "while")) {
            if (while_depth >= while_stack_max) {
                while_stack_max += 32;
                size_t* tmp = realloc(while_stack, while_stack_max * sizeof(size_t));
                if (tmp) while_stack = tmp;
            }
            while_stack[while_depth++] = i;
            if (!eval_conditions(args + 1)) {
                int wl = 1;
                for (i++; lines[i]; i++) {
                    if (strlen(lines[i]) == 0) continue;
                    if (startswith(lines[i], "while ")) wl++;
                    if (startswith(lines[i], "endwhile")) {
                        wl--;
                        if (wl == 0) break;
                    }
                }
                while_depth--;
            }
            goto args_cleanup;
        }
        if (iseq(args[0], "endwhile")) {
            if (while_depth > 0) {
                i = while_stack[--while_depth] - 1;
            }
            goto args_cleanup;
        }
        if (iseq(args[0], "goto")) {
            if (args[1]) {
                if (ret_depth >= ret_stack_max) {
                    ret_stack_max += 32;
                    size_t* tmp = realloc(ret_stack, ret_stack_max * sizeof(size_t));
                    if (tmp) ret_stack = tmp;
                }
                ret_stack[ret_depth++] = i;
                for (size_t li = 0; li < label_cur; li++) {
                    if (iseq(labels[li].name, args[1])) {
                        i = labels[li].line - 1;
                        break;
                    }
                }
            }
            goto args_cleanup;
        }
        if (iseq(args[0], "ret")) {
            if (ret_depth > 0) {
                i = ret_stack[--ret_depth] - 1;
            }
            goto args_cleanup;
        }
        if (iseq(args[0], "read")) {
            char buf[1024];
            if (fgets(buf, sizeof(buf), stdin)) {
                size_t blen = strlen(buf);
                while (blen > 0 && (buf[blen-1] == '\n' || buf[blen-1] == '\r')) buf[--blen] = '\0';
                if (args[1]) set_value(args[1], buf);
            }
            goto args_cleanup;
        }
        if (iseq(args[0], "exit")) {
            if (args[1]) rc = atoi(args[1]);
            else rc = 0;
            for(size_t j=0; args[j]; j++) free(args[j]);
            free(args);
            break;
        }
        rc = operation_main(global->manager, args[0], args+1);
args_cleanup:
        for(size_t j=0; args[j]; j++) free(args[j]);
        free(args);
        continue;
args_cleanup_err:
        for(size_t j=0; args[j]; j++) free(args[j]);
        free(args);
        return 1;
    }
    labels_unref();
    if (while_stack) {
        free(while_stack);
        while_stack = NULL;
        while_stack_max = 32;
        while_depth = 0;
    }
    if (ret_stack) {
        free(ret_stack);
        ret_stack = NULL;
        ret_stack_max = 32;
        ret_depth = 0;
    }
    for(size_t i=0; lines[i]; i++) free(lines[i]);
    free(lines);
    return rc;
}
