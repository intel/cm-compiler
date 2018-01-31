In order to use Makefile.linux in a customized environment, you may need to
adjust CM_ROOT and CMC setting to find your cmc executable and cmrt library.

after that, do the following:

  $ cd linear_walker
  $ make -f ../Makefile.linux
  $ hw_x64.linear_walker 
