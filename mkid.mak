ToolMkId = java -jar ../game/sys/jdeclareiid.jar ../game/sys/iddb.dat

../game/sys/iddb.dat : shared\*.h
    @set path=../game/sys/jre/bin;%path%
    @echo Building Iddb.dat for Shared
    @$(ToolMkId) shared -saveold .h
    @echo Building Iddb.dat for UI_Ingame
    @$(ToolMkId) ui_ingame -saveold .events.h

clean:
    @if exist ..\game\sys\iddb.dat del ..\game\sys\iddb.dat
