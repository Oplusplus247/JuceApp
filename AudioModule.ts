import { NativeModules } from 'react-native';

interface AudioModuleInterface {
  getVersion(): Promise<string>;
  testBridge(message: string): Promise<string>;
}

const { AudioModule } = NativeModules;

export default AudioModule as AudioModuleInterface;