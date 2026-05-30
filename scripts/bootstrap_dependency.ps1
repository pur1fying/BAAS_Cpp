param(
    [Parameter(ValueFromRemainingArguments = $true)]
    [string[]] $Args
)

$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$RepoRoot = Split-Path -Parent $ScriptDir

Push-Location $RepoRoot
python -m deploy.bootstrap_dependency @Args
$ExitCode = $LASTEXITCODE
Pop-Location
exit $ExitCode
