@echo off
rem START or STOP Services
rem ----------------------------------
rem Check if argument is STOP or START

if not ""%1"" == ""START"" goto stop

if exist C:\XAMPP\hypersonic\scripts\ctl.bat (start /MIN /B C:\XAMPP\server\hsql-sample-database\scripts\ctl.bat START)
if exist C:\XAMPP\ingres\scripts\ctl.bat (start /MIN /B C:\XAMPP\ingres\scripts\ctl.bat START)
if exist C:\XAMPP\mysql\scripts\ctl.bat (start /MIN /B C:\XAMPP\mysql\scripts\ctl.bat START)
if exist C:\XAMPP\postgresql\scripts\ctl.bat (start /MIN /B C:\XAMPP\postgresql\scripts\ctl.bat START)
if exist C:\XAMPP\apache\scripts\ctl.bat (start /MIN /B C:\XAMPP\apache\scripts\ctl.bat START)
if exist C:\XAMPP\openoffice\scripts\ctl.bat (start /MIN /B C:\XAMPP\openoffice\scripts\ctl.bat START)
if exist C:\XAMPP\apache-tomcat\scripts\ctl.bat (start /MIN /B C:\XAMPP\apache-tomcat\scripts\ctl.bat START)
if exist C:\XAMPP\resin\scripts\ctl.bat (start /MIN /B C:\XAMPP\resin\scripts\ctl.bat START)
if exist C:\XAMPP\jetty\scripts\ctl.bat (start /MIN /B C:\XAMPP\jetty\scripts\ctl.bat START)
if exist C:\XAMPP\subversion\scripts\ctl.bat (start /MIN /B C:\XAMPP\subversion\scripts\ctl.bat START)
rem RUBY_APPLICATION_START
if exist C:\XAMPP\lucene\scripts\ctl.bat (start /MIN /B C:\XAMPP\lucene\scripts\ctl.bat START)
if exist C:\XAMPP\third_application\scripts\ctl.bat (start /MIN /B C:\XAMPP\third_application\scripts\ctl.bat START)
goto end

:stop
echo "Stopping services ..."
if exist C:\XAMPP\third_application\scripts\ctl.bat (start /MIN /B C:\XAMPP\third_application\scripts\ctl.bat STOP)
if exist C:\XAMPP\lucene\scripts\ctl.bat (start /MIN /B C:\XAMPP\lucene\scripts\ctl.bat STOP)
rem RUBY_APPLICATION_STOP
if exist C:\XAMPP\subversion\scripts\ctl.bat (start /MIN /B C:\XAMPP\subversion\scripts\ctl.bat STOP)
if exist C:\XAMPP\jetty\scripts\ctl.bat (start /MIN /B C:\XAMPP\jetty\scripts\ctl.bat STOP)
if exist C:\XAMPP\hypersonic\scripts\ctl.bat (start /MIN /B C:\XAMPP\server\hsql-sample-database\scripts\ctl.bat STOP)
if exist C:\XAMPP\resin\scripts\ctl.bat (start /MIN /B C:\XAMPP\resin\scripts\ctl.bat STOP)
if exist C:\XAMPP\apache-tomcat\scripts\ctl.bat (start /MIN /B /WAIT C:\XAMPP\apache-tomcat\scripts\ctl.bat STOP)
if exist C:\XAMPP\openoffice\scripts\ctl.bat (start /MIN /B C:\XAMPP\openoffice\scripts\ctl.bat STOP)
if exist C:\XAMPP\apache\scripts\ctl.bat (start /MIN /B C:\XAMPP\apache\scripts\ctl.bat STOP)
if exist C:\XAMPP\ingres\scripts\ctl.bat (start /MIN /B C:\XAMPP\ingres\scripts\ctl.bat STOP)
if exist C:\XAMPP\mysql\scripts\ctl.bat (start /MIN /B C:\XAMPP\mysql\scripts\ctl.bat STOP)
if exist C:\XAMPP\postgresql\scripts\ctl.bat (start /MIN /B C:\XAMPP\postgresql\scripts\ctl.bat STOP)

:end

