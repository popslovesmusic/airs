param(
    [string] = "Debug",
    [string] = "all",           # use label (gw|satp|complex|ssp|ternary|basic) or 'all'
    [string] = "build",
    [string] = "Simulation",
    [switch],
    [string] = "Visual Studio 17 2022",
    [string] = "",
    [string] = ""
)

# Resolve paths
 = Split-Path -Parent System.Management.Automation.InvocationInfo.MyCommand.Path
 = (Resolve-Path "\..\").Path
 = Join-Path  
 = Join-Path  

# Default configure options
 = "-DBUILD_TESTS=ON -DBUILD_HARNESS=ON -DUSE_GTEST=ON -DFETCHCONTENT_SOURCE_DIR_GOOGLETEST=/_deps/googletest-src/googletest-1.14.0 -DFETCHCONTENT_UPDATES_DISCONNECTED=ON"

if( -or -not (Test-Path "/CMakeCache.txt")){
    Write-Host "[configure] cmake -S  -B  -G ''  "
    cmake -S  -B  -G ""  
    if( -ne 0){ exit  }
}

Write-Host "[build] cmake --build  --target harness_tests --config "
cmake --build  --target harness_tests --config 
if( -ne 0){ exit  }

 = if( -eq "all") { "harness" } else { "" }

Write-Host "[test] ctest -C  -L  "
Push-Location 
ctest -C  -L  
 = 
Pop-Location
exit 
