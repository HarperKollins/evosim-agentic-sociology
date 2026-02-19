$ErrorActionPreference = 'Stop'
$gpp = "c:\Users\ASUS\Desktop\collins universe\EvoSim\w64devkit\bin\g++.exe" 
$src = "c:\Users\ASUS\Desktop\collins universe\EvoSim\main.cpp"
$out = "c:\Users\ASUS\Desktop\collins universe\EvoSim\EvoSim.exe"
$inc = "c:\Users\ASUS\Desktop\collins universe\EvoSim"

Write-Host "Testing g++ version..."
$proc = Start-Process -FilePath $gpp -ArgumentList "--version" -NoNewWindow -Wait -PassThru -RedirectStandardOutput "$inc\version.txt" -RedirectStandardError "$inc\version_err.txt"
Get-Content "$inc\version.txt"
Get-Content "$inc\version_err.txt"

Write-Host "`nCompiling EvoSim..."
$proc = Start-Process -FilePath $gpp -ArgumentList "-std=c++17","-O0","-Wall","-o",$out,$src,"-I$inc" -NoNewWindow -Wait -PassThru -RedirectStandardOutput "$inc\build_out.txt" -RedirectStandardError "$inc\build_err.txt"

Write-Host "Exit code: $($proc.ExitCode)"
$errContent = Get-Content "$inc\build_err.txt" -Raw
if ($errContent) { Write-Host "ERRORS:`n$errContent" }

if ($proc.ExitCode -eq 0) { 
    Write-Host "`nBUILD SUCCESS! EvoSim.exe created." 
} else { 
    Write-Host "`nBUILD FAILED." 
}
