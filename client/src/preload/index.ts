import { contextBridge, ipcRenderer } from 'electron';

export interface CaptureSource {
  id: string;
  name: string;
  thumbnail: string;
}

export interface PlatformInfo {
  platform: string;
  arch: string;
  version: string;
}

export interface ElectronAPI {
  getSources: () => Promise<CaptureSource[]>;
  startSidecar: (port?: number) => Promise<{ status: string; port?: number; message?: string }>;
  stopSidecar: () => Promise<{ status: string }>;
  getPlatformInfo: () => Promise<PlatformInfo>;
}

const api: ElectronAPI = {
  getSources: () => ipcRenderer.invoke('get-sources'),
  startSidecar: (port?: number) => ipcRenderer.invoke('start-sidecar', port),
  stopSidecar: () => ipcRenderer.invoke('stop-sidecar'),
  getPlatformInfo: () => ipcRenderer.invoke('get-platform-info'),
};

contextBridge.exposeInMainWorld('passTheController', api);

declare global {
  interface Window {
    passTheController?: ElectronAPI;
  }
}
