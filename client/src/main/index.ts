import { app, BrowserWindow, ipcMain, desktopCapturer } from 'electron';
import * as path from 'path';
import { spawn, ChildProcess } from 'child_process';

let mainWindow: BrowserWindow | null = null;
let sidecarProcess: ChildProcess | null = null;

function createWindow() {
  mainWindow = new BrowserWindow({
    width: 1280,
    height: 800,
    minWidth: 1024,
    minHeight: 700,
    title: 'Pass the Controller',
    frame: true,
    backgroundColor: '#12141a',
    webPreferences: {
      preload: path.join(__dirname, '../preload/index.js'),
      nodeIntegration: false,
      contextIsolation: true,
      sandbox: false,
    },
  });

  const devUrl = process.env.VITE_DEV_SERVER_URL || 'http://localhost:5173';
  if (process.env.NODE_ENV === 'development' || !app.isPackaged) {
    mainWindow.loadURL(devUrl).catch(() => {
      mainWindow?.loadFile(path.join(__dirname, '../../dist/index.html'));
    });
  } else {
    mainWindow.loadFile(path.join(__dirname, '../../dist/index.html'));
  }

  mainWindow.on('closed', () => {
    mainWindow = null;
  });
}

// 1. IPC Handler: Enumerate screen / window capture sources for host streaming
ipcMain.handle('get-sources', async () => {
  try {
    const sources = await desktopCapturer.getSources({
      types: ['window', 'screen'],
      thumbnailSize: { width: 320, height: 180 },
      fetchWindowIcons: true,
    });
    return sources.map((s) => ({
      id: s.id,
      name: s.name,
      thumbnail: s.thumbnail.toDataURL(),
    }));
  } catch (err) {
    console.error('Failed to get capture sources:', err);
    return [];
  }
});

// 2. IPC Handler: Manage Native Input Injection Sidecar
ipcMain.handle('start-sidecar', async (_event, port = 9876) => {
  if (sidecarProcess) {
    return { status: 'already_running', port };
  }

  // Determine sidecar script / binary path
  const isWindows = process.platform === 'win32';
  const repoRoot = path.resolve(__dirname, '../../../');
  const mockScript = path.join(repoRoot, 'sidecar', 'agent_mock.py');
  const winScript = path.join(repoRoot, 'sidecar', 'agent_win.py');
  const scriptToRun = isWindows ? winScript : mockScript;

  console.log(`[Host Engine] Launching input sidecar: ${scriptToRun}`);

  try {
    sidecarProcess = spawn('python3', [scriptToRun, '--port', String(port)], {
      stdio: ['ignore', 'pipe', 'pipe'],
    });

    sidecarProcess.stdout?.on('data', (data) => {
      console.log(`[Sidecar STDOUT] ${data.toString().trim()}`);
    });

    sidecarProcess.stderr?.on('data', (data) => {
      console.error(`[Sidecar STDERR] ${data.toString().trim()}`);
    });

    sidecarProcess.on('exit', (code) => {
      console.log(`[Sidecar] Exited with code ${code}`);
      sidecarProcess = null;
    });

    return { status: 'started', port };
  } catch (err: any) {
    console.error('Failed to spawn sidecar:', err);
    return { status: 'error', message: err.message };
  }
});

ipcMain.handle('stop-sidecar', async () => {
  if (sidecarProcess) {
    sidecarProcess.kill();
    sidecarProcess = null;
    return { status: 'stopped' };
  }
  return { status: 'not_running' };
});

ipcMain.handle('get-platform-info', () => {
  return {
    platform: process.platform,
    arch: process.arch,
    version: app.getVersion(),
  };
});

app.whenReady().then(createWindow);

app.on('window-all-closed', () => {
  if (sidecarProcess) {
    sidecarProcess.kill();
  }
  if (process.platform !== 'darwin') {
    app.quit();
  }
});

app.on('activate', () => {
  if (BrowserWindow.getAllWindows().length === 0) {
    createWindow();
  }
});
