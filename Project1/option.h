int verbose_flag;
int profile_flag;

static struct option long_options[] = {
  {"rdonly",  required_argument, NULL,  3 },
  {"wronly",  required_argument, NULL,  4 },
  {"command", required_argument, NULL,  5 },
  {"rdwr", required_argument, NULL, 6},
  {"pipe", no_argument, NULL, 7},
  {"wait", no_argument, NULL, 8},
  {"close", required_argument, NULL, 9},
  {"abort", no_argument,NULL, 10},
  {"catch", required_argument, NULL, 11},
  {"ignore", required_argument, NULL, 12},
  {"default", required_argument, NULL, 13},
  {"pause", no_argument, NULL, 14},
  {"append", no_argument, NULL, 15},
  {"cloexec", no_argument, NULL, 16},
  {"creat", no_argument, NULL, 17},
  {"directory", no_argument, NULL, 18},
  {"dsync", no_argument, NULL, 19},
  {"excl", no_argument, NULL, 20},
  {"nofollow", no_argument, NULL, 21},
  {"nonblock", no_argument, NULL, 22},
  {"rsync", no_argument, NULL, 23},
  {"sync", no_argument, NULL, 24},
  {"trunc", no_argument, NULL, 25},
  {"profile", no_argument, &profile_flag, 1},
  {"verbose", no_argument, &verbose_flag,  1 },//this option set the verbose flag
  {0,         0,                 0,  0 }
};
