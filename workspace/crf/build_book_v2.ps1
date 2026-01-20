$chapters = 1..28
$outputFile = "D:\airs\workspace\crf\Constraint_Resolution_Framework.html"

# CSS Styles
$css = @"
<style>
    body { font-family: 'Segoe UI', Roboto, Helvetica, Arial, sans-serif; line-height: 1.6; color: #333; max-width: 800px; margin: 0 auto; padding: 40px; background-color: #f9f9f9; }
    h1, h2, h3 { color: #111; margin-top: 1.5em; margin-bottom: 0.5em; font-weight: 600; }
    h1 { font-size: 2.2em; border-bottom: 2px solid #333; padding-bottom: 10px; }
    h2 { font-size: 1.8em; border-bottom: 1px solid #ddd; padding-bottom: 5px; }
    h3 { font-size: 1.4em; }
    p { margin-bottom: 1em; }
    ul { margin-bottom: 1em; padding-left: 20px; }
    li { margin-bottom: 0.5em; }
    code { background-color: #eee; padding: 2px 4px; border-radius: 3px; font-family: Consolas, monospace; font-size: 0.9em; }
    pre { background-color: #2d2d2d; color: #f8f8f2; padding: 15px; border-radius: 5px; overflow-x: auto; font-family: Consolas, monospace; }
    blockquote { border-left: 4px solid #333; margin: 0; padding-left: 15px; color: #555; font-style: italic; }
    .chapter-break { margin-top: 60px; margin-bottom: 60px; border-top: 1px dashed #ccc; }
    .toc { background: #fff; padding: 20px; border: 1px solid #ddd; border-radius: 5px; margin-bottom: 40px; }
    .toc h2 { margin-top: 0; font-size: 1.5em; border: none; }
    .toc ul { list-style-type: none; padding-left: 0; }
    .toc a { text-decoration: none; color: #0066cc; }
    .toc a:hover { text-decoration: underline; }
</style>
"@

$htmlHeader = @"
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Constraint Resolution Framework</title>
    $css
</head>
<body>
    <div class="toc">
        <h2>Table of Contents</h2>
        <ul>
"@

$htmlFooter = @"
</body>
</html>
"@

function Convert-MarkdownToHtml {
    param ($text)
    
    $text = $text.Trim() # Remove leading/trailing newlines
    $text = $text -replace "&", "&amp;" -replace "<", "&lt;" -replace ">", "&gt;"
    
    # Headers
    $text = $text -replace '(?m)^# (.*)$', '<h1>$1</h1>'
    $text = $text -replace '(?m)^## (.*)$', '<h2>$1</h2>'
    $text = $text -replace '(?m)^### (.*)$', '<h3>$1</h3>'
    
    # Bold / Italic
    $text = $text -replace '\*\*(.*?)\*\*', '<strong>$1</strong>'
    $text = $text -replace '\*(.*?)\*', '<em>$1</em>'
    
    # Lists
    $text = $text -replace '(?m)^\* (.*)$', '<li>$1</li>'
    
    # Code
    $text = $text -replace '`(.*?)`', '<code>$1</code>'
    
    # Line processing
    $lines = $text -split "\r?\n"
    $newLines = @()
    $inList = $false
    
    foreach ($line in $lines) {
        $trimmed = $line.Trim()
        
        if ([string]::IsNullOrWhiteSpace($trimmed)) {
            if ($inList) { $newLines += "</ul>"; $inList = $false }
            continue 
        }
        
        if ($line -match '^<h[1-6]>') {
            if ($inList) { $newLines += "</ul>"; $inList = $false }
            $newLines += $line
        }
        elseif ($line -match '^<li>') {
            if (-not $inList) { $newLines += "<ul>"; $inList = $true }
            $newLines += $line
        }
        else {
            if ($inList) { $newLines += "</ul>"; $inList = $false }
            $newLines += "<p>$line</p>"
        }
    }
    if ($inList) { $newLines += "</ul>" }
    
    return $newLines -join "`n"
}

$tocContent = ""
$bodyContent = ""

foreach ($i in $chapters) {
    $filename = "D:\airs\workspace\crf\chapter_$i.md"
    if (Test-Path $filename) {
        Write-Host "Processing $filename..."
        $rawMd = Get-Content $filename -Raw -Encoding UTF8
        
        # Use regex with multiline mode (?m) to find header even if not on first line
        if ($rawMd -match '(?m)^# (.*)$') {
            $title = $matches[1].Trim()
            $id = "chapter-$i"
            $tocContent += "<li><a href='#$id'>$title</a></li>`n"
            
            $convertedHtml = Convert-MarkdownToHtml $rawMd
            # Inject ID into the H1
            $convertedHtml = $convertedHtml -replace '<h1>(.*?)</h1>', "<h1 id='$id'>`$1</h1>"
            
            $bodyContent += "<div class='chapter-break'></div>`n" + $convertedHtml
        } else {
            Write-Host "WARNING: No header found in $filename"
        }
    } else {
        Write-Host "ERROR: File not found: $filename"
    }
}

$tocContent += "</ul></div>"
$finalHtml = $htmlHeader + $tocContent + $bodyContent + $htmlFooter
$finalHtml | Out-File -FilePath $outputFile -Encoding UTF8

Write-Host "Book generated at: $outputFile"
