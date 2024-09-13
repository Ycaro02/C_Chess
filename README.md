# C_Chess: A High-Performance Chess Engine

C_Chess is a high-performance chess engine written in C, utilizing SDL2 for graphics and bitboards for board representation to optimize move calculations. The project is designed to be cross-platform, initially supporting Unix-like systems, and later extended to Windows and Android. A WebAssembly version using Emscripten is currently in development. The latest releases for Windows and Android are available on the repository.

## Technical Choices

- **Bitboard Representation**: Efficiently represent the chessboard using bitboards to optimize move calculations.
- **Cross-Platform**: Designed to run on Unix-like systems, Windows, and Android.
- **Graphics Rendering**: Utilizes SDL2 for rendering the chessboard and pieces.
- **Move Calculation**: Optimized move calculations for high performance.
- **WebAssembly**: A WebAssembly version is in progress using Emscripten.

## Usage

### To build the program on unix like system, follow these steps:

1. **Clone the Repository**:
   ```bash
   git clone https://github.com/Ycaro02/C_Chess.git --recursive
   ```

2. **Build the Project**:
   ```bash
	cd C_Chess
	make
	./C_chess
   ```

### For Windows, follow these steps:

1 **Download the Release**:

Go to the releases page and download the latest Windows release.
Run the Installer (.exe file):

The release contains an installer executable created using Inno Setup. Run the installer and follow the on-screen instructions to install C_Chess on your system.

2 **Launch the Program**:

After installation, you can launch C_Chess from the Start Menu or the installation directory.

### For Android, follow these steps:

1 **Download the APK**:

Go to the releases page and download the latest Android APK from your Android device or another device.

2 **Install the APK**:

Transfer the APK to your Android device.
Open the APK file on your device and follow the on-screen instructions to install the application.

3 **Launch the App**:
After installation, you can launch C_Chess from your app drawer.