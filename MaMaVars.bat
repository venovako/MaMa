@ECHO OFF
IF CMDEXTVERSION 1 GOTO testvar
GOTO setvar
:testvar
IF DEFINED MAMA_HOME GOTO setpath
:setvar
SET MAMA_HOME=%CD%
:setpath
SET Path=%MAMA_HOME%\bin;%Path%
