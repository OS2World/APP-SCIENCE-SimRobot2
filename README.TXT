Hello SimRobot User!

You need Borland C++ 2.x for OS/2 to compile SimRobot.
You must have installed the OWL 2.0 (or higher).

IDE:
- Open SRLIB.PRJ with the IDE.
- Select Project|View Settings|Directories and enter the directories used
  by your installation.
- Build SRLIB.
- Open SIMROBOT.PRJ with the IDE.
- Select Project|View Settings|Directories and enter the directories used
  by your installation.
- Build SimRobot.

Command line:
- make /fSimRobot.mak SimRobot.exe

If you write your own controller files, add them to
SIMROBOT.PRJ, because then they will contain debugging
information.
For information on robot controllers look into online help.

If there are any problems, read our www-page
  http:///www.informatik.uni-bremen.de/~simrobot
and possibly mail us to
  simrobot@informatik.uni-bremen.de

Have fun!