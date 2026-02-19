# Manual Setup Guide for EvoSim

If the automated setup fails, you will need to install a C++ compiler manually. Here are two ways to do it.

## Option 1: Install MinGW (Recommended & Lightweight)

1.  **Download w64devkit**:
    *   Go to: [w64devkit Releases](https://github.com/skeeto/w64devkit/releases)
    *   Download `w64devkit-x64-1.20.0.zip` (or latest).
2.  **Extract**:
    *   Unzip the file to `C:\w64devkit`.
3.  **Add to PATH**:
    *   Press `Win + R`, type `sysdm.cpl`, press Enter.
    *   Go to **Advanced** tab -> **Environment Variables**.
    *   Under **System variables**, select **Path** and click **Edit**.
    *   Click **New** and add: `C:\w64devkit\bin`.
    *   Click OK on all windows.
4.  **Verify**:
    *   Open a new terminal and run: `g++ --version`
5.  **Build**:
    *   Run `build.bat` in the project folder.

## Option 2: Install Visual Studio C++ Tools

If you already have Visual Studio installed but see build errors:

1.  Open **Visual Studio Installer** from Start Menu.
2.  Click **Modify** on your Visual Studio installation.
3.  Select the **Desktop development with C++** workload.
4.  Click **Modify** / **Install** to download the tools.
5.  Once done, open this project folder again and run `build_msvc.bat`.

## Option 3: Install Scoop (For Power Users)

1.  Open PowerShell.
2.  Run: `Set-ExecutionPolicy RemoteSigned -Scope CurrentUser`
3.  Run: `irm get.scoop.sh | iex`
4.  Run: `scoop install gcc`
