/**
 * @brief Parses command-line arguments.
 *
 * This function takes an array of command-line arguments and processes them.
 * It may modify the input array or return a new array of parsed arguments.
 *
 * @param args A pointer to an array of strings (char**) representing the command-line arguments.
 *             The first element is typically the name of the program.
 * @return A pointer to an array of strings (char**) containing the parsed arguments.
 *         The caller is responsible for freeing the returned array.
 *         Returns NULL if parsing fails or if args is NULL.
 */
char** parse_args(char** args);

/**
 * @brief Executes a script file.
 *
 * This function runs a script specified by the given file path. It reads the script,
 * interprets its contents, and executes the commands within it.
 *
 * @param script A pointer to a string (const char*) representing the path to the script file.
 *               The script must be accessible and readable.
 * @return An integer indicating the success or failure of the script execution.
 *         Returns 0 on success, or a non-zero error code on failure.
 */
int run_script(const char* script);
