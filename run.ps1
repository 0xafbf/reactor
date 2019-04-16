
Param(
	[switch] $Debug = $false
	)

if ($Debug) {
	& devenv.exe -NoSplash -Run reactor.sln -DebugExe bin/work_x64/reactor.exe
	exit 0
}

& bin/work_x64/reactor.exe
