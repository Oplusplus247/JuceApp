// AudioModule.ts
import { NativeModules } from 'react-native';

type AudioModuleType = {
  init(): Promise<boolean>;
  shutdown(): Promise<boolean>;
  version(): Promise<string>;
  sampleRate(): Promise<number>;
  setJuceEnabled(enabled: boolean): void;
};

const { AudioModule } = NativeModules;

// Ensure we crash loudly if the native module isn't linked
if (!AudioModule) {
  throw new Error(
    'AudioModule native module not found. ' +
      'Did you rebuild the app after adding native code?'
  );
}

export default AudioModule as AudioModuleType;