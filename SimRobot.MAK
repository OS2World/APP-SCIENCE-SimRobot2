#=============================================================
#
#	SIMROBOT.MAK - Makefile for project srLib.prj and
#                      SimRobot.prj
#		Created on 05/09/95 at 14:55
#
#=============================================================

.AUTODEPEND

.PATH.obj = .\obj

#=============================================================
#		Translator Definitions
#=============================================================
CC = bcc +srLib.cfg
# For controller files use other parameters
CCC = bcc +SIMROBOT.CFG

TASM = tasm.exe
TLIB = tlib.exe
TLINK = tlink
RC = brcc.exe
RB = rc.exe
LIBPATH = D:\BCOS2\LIB
INCLUDEPATH = D:\BCOS2\INCLUDE

#=============================================================
#		Implicit Rules
#=============================================================
.c.obj:
  @$(CC) -c {$< }

.cpp.obj:
  @$(CC) -c {$< }

.asm.obj:
  @$(TASM) -Mx $*.asm,$*.obj

.rc.res:
  @$(RC) -r $*.rc

#=============================================================
#		List Macros
#=============================================================
LINK_EXCLUDE =  \
simrobot.res

LINK_INCLUDE =  \
 srlib.lib \
 robotarm.obj \
 motion.obj

LIB_DEPENDENCIES =  \
 zbuffer.obj \
 vector.obj \
 sim3dworld.obj \
 sim3dvehicle.obj \
 sim3dsensors.obj \
 sim3dreader.obj \
 sim3dportdef.obj \
 sim3dmoveable.obj \
 sim3dfile.obj \
 sim3derrors.obj \
 sim3demitter.obj \
 sim3ddynarray.obj \
 sim3dbody.obj \
 sim3dbasics.obj \
 sim3d.obj \
 sensorclasses.obj \
 nonportable.obj \
 mystring.obj \
 hiddengraph.obj \
 geometry.obj \
 basicgraphics.obj \
 controller.obj \
 sractorview.obj \
 srsensorview.obj \
 srobjectview.obj \
 srdataview.obj \
 srtreeview.obj \
 srbaseview.obj \
 sreditor.obj \
 srdoc.obj \
 srapp.obj

#=============================================================
#		Explicit Rules
#=============================================================

srlib.lib: srlib.cfg $(LIB_DEPENDENCIES)
  @- del srlib.lib
  @$(TLIB) $<  @&&|
+.\obj\zbuffer.obj &
+.\obj\vector.obj &
+.\obj\sim3dworld.obj &
+.\obj\sim3dvehicle.obj &
+.\obj\sim3dsensors.obj &
+.\obj\sim3dreader.obj &
+.\obj\sim3dportdef.obj &
+.\obj\sim3dmoveable.obj &
+.\obj\sim3dfile.obj &
+.\obj\sim3derrors.obj &
+.\obj\sim3demitter.obj &
+.\obj\sim3ddynarray.obj &
+.\obj\sim3dbody.obj &
+.\obj\sim3dbasics.obj &
+.\obj\sim3d.obj &
+.\obj\sensorclasses.obj &
+.\obj\nonportable.obj &
+.\obj\mystring.obj &
+.\obj\hiddengraph.obj &
+.\obj\geometry.obj &
+.\obj\basicgraphics.obj &
+.\obj\controller.obj &
+.\obj\sractorview.obj &
+.\obj\srsensorview.obj &
+.\obj\srobjectview.obj &
+.\obj\srdataview.obj &
+.\obj\srtreeview.obj &
+.\obj\srbaseview.obj &
+.\obj\sreditor.obj &
+.\obj\srdoc.obj &
+.\obj\srapp.obj
|

SimRobot.exe: simrobot.cfg $(LINK_INCLUDE) $(LINK_EXCLUDE)
  @$(TLINK) /v /c /x /Toe /aa /L$(LIBPATH) @&&|
D:\BCOS2\LIB\C02.OBJ+
.\obj\robotarm.obj+
.\obj\motion.obj
SimRobot
	# no map
srlib.lib+
D:\BCOS2\LIB\BIDS2I.LIB+
D:\BCOS2\LIB\OWLPMI.LIB+
D:\BCOS2\LIB\C2I.LIB+
D:\BCOS2\LIB\OS2.LIB
|
  @rc.exe SimRobot.RES SimRobot.exe

all: SimRobot.exe

#=============================================================
#		Individual File Dependencies
#=============================================================
SimRobot.RES: simrobot.cfg SIMROBOT.RC
	@brcc.exe -R -I$(INCLUDEPATH) -FO SimRobot.RES SIMROBOT.RC

RobotArm.obj: simrobot.cfg Examples\RobotArm.cpp
	@$(CCC) -c Examples\RobotArm.cpp

Motion.obj: simrobot.cfg Examples\Motion.cpp
	@$(CCC) -c Examples\Motion.cpp

#=============================================================
#		Individual File Dependencies
#=============================================================
ZBuffer.obj: srlib.cfg ZBuffer.cpp

Vector.obj: srlib.cfg Vector.cpp

sim3DWorld.obj: srlib.cfg sim3DWorld.cpp

sim3DVehicle.obj: srlib.cfg sim3DVehicle.cpp

sim3DSensors.obj: srlib.cfg sim3DSensors.cpp

sim3DReader.obj: srlib.cfg sim3DReader.cpp

sim3DPortDef.obj: srlib.cfg sim3DPortDef.cpp

sim3DMoveable.obj: srlib.cfg sim3DMoveable.cpp

sim3DFile.obj: srlib.cfg sim3DFile.cpp

sim3DErrors.obj: srlib.cfg sim3DErrors.cpp

sim3DEmitter.obj: srlib.cfg sim3DEmitter.cpp

sim3DDynArray.obj: srlib.cfg sim3DDynArray.cpp

sim3DBody.obj: srlib.cfg sim3DBody.cpp

sim3DBasics.obj: srlib.cfg sim3DBasics.cpp

sim3D.obj: srlib.cfg sim3D.cpp

SensorClasses.obj: srlib.cfg SensorClasses.cpp

NonPortable.obj: srlib.cfg NonPortable.cpp

mystring.obj: srlib.cfg mystring.cpp

HiddenGraph.obj: srlib.cfg HiddenGraph.cpp

Geometry.obj: srlib.cfg Geometry.cpp

BasicGraphics.obj: srlib.cfg BasicGraphics.cpp

Controller.obj: srlib.cfg Controller.cpp

srActorView.obj: srlib.cfg srActorView.cpp

srSensorView.obj: srlib.cfg srSensorView.cpp

srObjectView.obj: srlib.cfg srObjectView.cpp

srDataView.obj: srlib.cfg srDataView.cpp

srTreeView.obj: srlib.cfg srTreeView.cpp

srBaseView.obj: srlib.cfg srBaseView.cpp

srEditor.obj: srlib.cfg srEditor.cpp

srDoc.obj: srlib.cfg srDoc.cpp

srApp.obj: srlib.cfg srApp.cpp

#=============================================================
#	   Compiler Configuration File for srLib.lib
#=============================================================
srlib.cfg:
  @copy >nul &&|
-xf
-Oi
-Oa
-Oz
-Ob
-Oe
-Oc
-D_RTLDLL;_BIDSDLL;_OWLDLL
-L$(LIBPATH)
-I$(INCLUDEPATH)
-n.\obj
-vi-
-k-
-O
-Ot
-w-inl
| srlib.cfg

#=============================================================
#	  Compiler Configuration File for SimRobot.exe
#=============================================================
simrobot.cfg: simrobot.mak
  @copy >nul &&|
-xf
-Oi
-Oa
-Oz
-Ob
-Oe
-Oc
-D_RTLDLL;_BIDSDLL;_OWLDLL
-L$(LIBPATH)
-I$(INCLUDEPATH)
-n.\obj
-vi-
-k-
-O
-Ot
-v
-w-inl
| simrobot.cfg

