# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## High-Level Architecture

This is a Windows MFC (Microsoft Foundation Classes) application for industrial process control, specifically for steel manufacturing lines. The application is structured as follows:

1. **Main Application Layer**: Uses MFC framework with a dialog-based UI (`SRV\SRV_Server.cpp`, `SRV\SRV_Dlg.cpp`)
2. **Threading Model**: Multi-threaded architecture with dedicated threads for different subsystems:
   - MAIN Thread: Core application logic
   - SQL Thread: Database operations
   - HMI Thread: Human-Machine Interface communication
   - SPG Thread: Process control calculations
   - SIM Thread: Simulation mode operations
3. **Database Layer**: Uses ADO (ActiveX Data Objects) for SQL Server connectivity (`SQL\SQL.cpp`)
4. **Process Control Modules**:
   - RTF (Radiant Tube Furnace) control
   - SLW (Slow Cooling) control
   - JET (Jet Cooling) control
   - OVG (Overaging) control
   - FCS (Final Cooling) control
5. **Configuration System**: Site-specific configurations in `Project Specific\CISDI\Sources`

## Common Development Tasks

### Building the Project
Since this is a legacy MFC project, it should be built using Visual Studio:
```
# Open the project in Visual Studio and build using the IDE
# Or use MSBuild if a solution file is available
msbuild ServerSocket.sln /p:Configuration=Release
```

### Running the Application
```
# Run the compiled executable
LtopServer.exe
```

### Testing
This appears to be an industrial control system without unit tests. Testing would involve:
1. Running the application in simulation mode
2. Verifying database connectivity
3. Checking process control calculations

### Code Structure
- `Headers\`: Global header files
- `SRV\`: Server and UI components
- `SQL\`: Database connectivity
- `SPG\`: Process control generation
- `SPS\`: Process control supervision
- `UTL\`: Utility functions
- `SIM\`: Simulation components
- `Project Specific\`: Site-specific configurations and resources

## Important Notes

1. This is a Windows-only application using MFC and ADO
2. Database connectivity uses SQL Server with specific connection strings in configuration files
3. The application supports multiple operational modes (Simulation/Production)
4. Thread safety is implemented using critical sections
5. Configuration is site-specific and loaded from data files