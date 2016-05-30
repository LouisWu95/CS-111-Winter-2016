void print_verbose(int argc, char **argv, int index)
{
  printf ("%s", argv[index]);
  int i = index+1;
  while (i<argc)
    {
      if(argv[i][0]== '-' && argv[i][1]=='-')  
	   break;
      printf(" %s", argv[i]); 
      i++;
    }
  printf("\n");
}
