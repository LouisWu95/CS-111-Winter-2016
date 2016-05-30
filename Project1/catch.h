//this header file does the signal handling part
void simpsh_handler_catch(int signum)
{
  fprintf(stderr, "%d caught\n", signum);
  exit(signum);
}
