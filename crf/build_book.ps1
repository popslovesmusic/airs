
$chapters = 1..28
$outputFile = "D:\airs\workspace\crf\Constraint_Resolution_Framework.html"

$css = @"
<style>
    body {
        font-family: 'Segoe UI', Roboto, Helvetica, Arial, sans-serif;
        line-height: 1.6;
        color: #333;
        max-width: 800px;
        margin: 0 auto;
        padding: 40px;
        background-color: #f9f9f9;
    }
    h1, h2, h3 {
        color: #111;
        margin-top: 1.5em;
        margin-bottom: 0.5em;
        font-weight: 600;
    }
    h1 { font-size: 2.2em; border-bottom: 2px solid #333; padding-bottom: 10px; }
    h2 { font-size: 1.8em; border-bottom: 1px solid #ddd; padding-bottom: 5px; }
    h3 { font-size: 1.4em; }
    p { margin-bottom: 1em; }
    ul { margin-bottom: 1em; padding-left: 20px; }
    li { margin-bottom: 0.5em; }
    code {
        background-color: #eee;
        padding: 2px 4px;
        border-radius: 3px;
        font-family: Consolas, monospace;
        font-size: 0.9em;
    }
    pre {
        background-color: #2d2d2d;
        color: #f8f8f2;
        padding: 15px;
        border-radius: 5px;
        overflow-x: auto;
        font-family: Consolas, monospace;
    }
    blockquote {
        border-left: 4px solid #333;
        margin: 0;
        padding-left: 15px;
        color: #555;
        font-style: italic;
    }
    .chapter-break {
        margin-top: 60px;
        margin-bottom: 60px;
        border-top: 1px dashed #ccc;
    }
    .toc {
        background: #fff;
        padding: 20px;
        border: 1px solid #ddd;
        border-radius: 5px;
        margin-bottom: 40px;
    }
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

# Helper function for basic MD to HTML
function Convert-MarkdownToHtml {
    param ($text)
    
    # Escape HTML
    $text = $text -replace "&", "&amp;" -replace "<", "&lt;" -replace ">", "&gt;"
    
    # Headers
    $text = $text -replace '(?m)^# (.*)$', '<h1>$1</h1>'
    $text = $text -replace '(?m)^## (.*)$', '<h2>$1</h2>'
    $text = $text -replace '(?m)^### (.*)$', '<h3>$1</h3>'
    
    # Bold / Italic
    $text = $text -replace '\*\*(.*?)\*\*', '<strong>$1</strong>'
    $text = $text -replace '\*(.*?)\*', '<em>$1</em>'
    
    # Lists (Basic handling)
    $text = $text -replace '(?m)^\* (.*)$', '<li>$1</li>'
    
    # Code blocks (Inline)
    $text = $text -replace '`(.*?)`', '<code>$1</code>'
    
    # Paragraphs: Wrap lines that aren't headers or list items in <p>
    # This is a bit tricky with Regex in one pass, so we split by newlines
    $lines = $text -split "`r`n|`n"
    $newLines = @()
    $inList = $false
    
    foreach ($line in $lines) {
        if ([string]::IsNullOrWhiteSpace($line)) {
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

# --- GENERATE CONTENT ---

$tocContent = ""
$bodyContent = ""

foreach ($i in $chapters) {
    $filename = "D:\airs\workspace\crf\chapter_$i.md"
    if (Test-Path $filename) {
        $rawMd = Get-Content $filename -Raw -Encoding UTF8
        
        # Extract Title for TOC
        if ($rawMd -match '^# (.*)$') {
            $title = $matches[1]
            $id = "chapter-$i"
            $tocContent += "<li><a href='#$id'>$title</a></li>`n"
            
            # Add ID to the H1 tag in the body for linking
            $convertedHtml = Convert-MarkdownToHtml $rawMd
            $convertedHtml = $convertedHtml -replace '<h1>(.*?)</h1>', "<h1 id='$id'>`$1</h1>"
            
            $bodyContent += "<div class='chapter-break'></div>`n" + $convertedHtml
        }
    }
}

$tocContent += "</ul></div>"

# Combine everything
$finalHtml = $htmlHeader + $tocContent + $bodyContent + $htmlFooter
$finalHtml | Out-File -FilePath $outputFile -Encoding UTF8

Write-Host "Book generated at: $outputFile"
