$modelsTarget = "D:\Prorgram\Project\ArcheoPhd\desktop\models"
$appDataDir = [System.IO.Path]::Combine($env:LOCALAPPDATA, 'ArchaeoPhD')
$modelsLink = Join-Path $appDataDir 'models'

if (-not (Test-Path $modelsLink)) {
    cmd /c "mklink /J `"$modelsLink`" `"$modelsTarget`""
    Write-Output "Created junction: $modelsLink -> $modelsTarget"
} else {
    Write-Output "Models already exists at: $modelsLink"
}

# Verify files accessible
$qwenPath = Join-Path $modelsLink 'llm\Qwen2.5-7B-Instruct-Q4_K_M.gguf'
$nomicPath = Join-Path $modelsLink 'embedding\nomic-embed-text-v1.5.Q4_K_M.gguf'

Write-Output ("Qwen GGUF present: {0}" -f (Test-Path $qwenPath))
Write-Output ("Nomic GGUF present: {0}" -f (Test-Path $nomicPath))

