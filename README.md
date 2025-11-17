# MiniSnip

MiniSnip is an extremely fast and lightweight screen capture and OCR tool for Windows. It's built with native C++ and designed to use only built-in Windows dependencies, resulting in a tiny, portable application that uses minimal resources.

It lives in your system tray, stays out of your way, and provides a modern workflow.

## Features

- **Feather-light:** The entire application is a single, tiny executable with no dependencies.
    
- **System Tray Native:** Lives in your system tray, using almost zero RAM.
    
- **Modern Workflow:** Use one hotkey (`Ctrl+Shift+C`) to snip _first_, then decide what to do.
    
- **Floating Action Toolbar:** An instant, dark-mode toolbar appears after your snip with four actions:
    
    - **Copy Image:** Copies the captured image to your clipboard.
        
    - **Save Image:** Opens a dialog to save the image as a PNG.
        
    - **Copy Text (OCR):** Uses the built-in Windows OCR engine to extract all text from the image and copy it to your clipboard.
        
    - **Save Text (OCR):** Appends the extracted text to a single `minisnip_OCR.txt` file in your Downloads folder. 
        
- **Zero Dependencies:** The portable version runs on Windows without installation.
    
- **Zero Data Collection:** Fully on-device. Nothing is ever sent over the network. 
    

## Installation

You have two options to get MiniSnip:

### 1. Recommended (Microsoft Store)

This is the easiest way to get MiniSnip with automatic updates.

### 2. Portable Version (GitHub)

This is a single `.exe` file that you can run from anywhere without installing.
    

   
    

## How to Use

1. Run `MiniSnip.exe`. A scissor icon will appear in your system tray.
    
2. Press **`Ctrl+Shift+C`** at any time. Your screen will dim.
    
3. Click and drag to select the area you want to capture.
    
4. When you release the mouse, a small floating toolbar will appear.
    
5. Click the action you want to perform.
    

Right-clicking the tray icon will also give you an option to "Start Snip" or "Exit" the application.

You can press `Esc` or right click at any time to cancel. 

## Building from Source

This project is a standard C++ Win32 application built with Visual Studio 2022.

### Requirements

- Visual Studio 2022
    
- The "Desktop development with C++" workload
    
- The "Universal Windows Platform development" workload (for the Windows OCR/WinRT dependencies)
    
- Windows 11 SDK
    

### Build Steps

1. Clone this repository: `git clone https://github.com/YOUR_USERNAME/MiniSnip.git`
    
2. Open the `MiniSnip.sln` solution file in Visual Studio.
    
3. Select the build configuration (e.g., `Release` and `x64`).
    
    

