// App.tsx
import React, { useEffect, useState } from 'react';
import { SafeAreaView, Text, Button, View, StyleSheet, PermissionsAndroid, Alert } from 'react-native';
import AudioModule from './AudioModule'; 

export default function App() {
  const [sr, setSr] = useState<number | null>(null);
  const [ver, setVer] = useState<string>('');
  const [isPlaying, setIsPlaying] = useState<boolean>(false);
  const [juceEnabled, setJuceEnabled] = useState(true);
  const [hasPermission, setHasPermission] = useState<boolean>(false);

  // Request microphone permission
  const requestMicrophonePermission = async () => {
    try {
      const granted = await PermissionsAndroid.request(
        PermissionsAndroid.PERMISSIONS.RECORD_AUDIO,
        {
          title: "Microphone Permission",
          message: "This app needs access to your microphone to process audio",
          buttonNeutral: "Ask Me Later",
          buttonNegative: "Cancel",
          buttonPositive: "OK"
        }
      );
      setHasPermission(granted === PermissionsAndroid.RESULTS.GRANTED);
      return granted === PermissionsAndroid.RESULTS.GRANTED;
    } catch (err) {
      console.warn('Permission error:', err);
      return false;
    }
  };

  useEffect(() => {
    (async () => {
      try {
        const [v, rate] = await Promise.all([
          AudioModule.version(),
          AudioModule.sampleRate(),
        ]);
        setVer(v);
        setSr(rate);
      } catch (e) {
        console.warn('Initial info fetch failed', e);
      }
    })();

    // Request permission on app start
    requestMicrophonePermission();

    return () => {
      AudioModule.shutdown().catch(() => {});
    };
  }, []);

  const handleToggleAudio = async () => {
    if (!hasPermission) {
      const granted = await requestMicrophonePermission();
      if (!granted) {
        Alert.alert("Permission Required", "Microphone permission is needed to process audio");
        return;
      }
    }

    if (isPlaying) {
      try {
        await AudioModule.shutdown();
        setIsPlaying(false);
        console.log("Audio stopped");
      } catch (e) {
        console.warn("Failed to stop audio", e);
      }
    } else {
      try {
        const success = await AudioModule.init();
        if (success) {
          setIsPlaying(true);
          console.log("Audio started - Mic -> Processing -> Speaker");
        } else {
          console.warn("Audio init returned false");
          Alert.alert("Error", "Failed to start audio processing");
        }
      } catch (e) {
        console.warn("Failed to start audio", e);
        Alert.alert("Error", "Failed to start audio processing");
      }
    }
  };

  const toggleJuceEffect = () => {
    const newState = !juceEnabled;
    setJuceEnabled(newState);
    AudioModule.setJuceEnabled(newState);
    console.log(`JUCE effects ${newState ? 'enabled' : 'disabled'}`);
  };

  return (
    <SafeAreaView style={styles.container}>
      <Text style={styles.title}>Audio Processing Test</Text>
      
      <View style={styles.infoContainer}>
        <Text style={styles.text}>Version: {ver || 'Loading...'}</Text>
        <Text style={styles.text}>Sample Rate: {sr ? `${sr}Hz` : 'Loading...'}</Text>
        <Text style={styles.text}>Status: {isPlaying ? '🎵 Processing Mic Input' : '⏹️ Stopped'}</Text>
        <Text style={styles.text}>Effects: {juceEnabled ? '🔊 ON (Ping-Pong)' : '🔇 OFF (Clean)'}</Text>
        <Text style={[styles.text, hasPermission ? styles.permissionGranted : styles.permissionDenied]}>
          Microphone Permission: {hasPermission ? '✅ Granted' : '❌ Not Granted'}
        </Text>
      </View>
      
      <View style={styles.buttonContainer}>
        <Button 
          title={isPlaying ? "Stop Processing" : "Start Processing"} 
          onPress={handleToggleAudio}
          color={isPlaying ? "#ff4444" : hasPermission ? "#44ff44" : "#888888"}
          disabled={!hasPermission && !isPlaying}
        />
        
        <View style={styles.spacer} />
        
        <Button 
          title={juceEnabled ? "Disable Effects" : "Enable Effects"} 
          onPress={toggleJuceEffect}
          color={juceEnabled ? "#8844ff" : "#4444ff"}
          disabled={!isPlaying}
        />
      </View>
      
      {!hasPermission && (
        <View style={styles.permissionWarning}>
          <Text style={styles.warningText}>
            ❗ Microphone permission required to process audio
          </Text>
        </View>
      )}
      
      <View style={styles.instructions}>
        <Text style={styles.instructionText}>
          {isPlaying 
            ? "🎤 Speak into your microphone to hear processed audio" 
            : "Press 'Start Processing' to begin"}
        </Text>
        <Text style={styles.instructionText}>
          The effect creates a volume modulation (ping-pong) when enabled
        </Text>
      </View>
    </SafeAreaView>
  );
}

const styles = StyleSheet.create({
  container: {
    flex: 1,
    padding: 24,
    backgroundColor: '#f5f5f5',
  },
  title: {
    fontSize: 24,
    fontWeight: 'bold',
    textAlign: 'center',
    marginBottom: 20,
    color: '#333',
  },
  infoContainer: {
    backgroundColor: 'white',
    padding: 16,
    borderRadius: 8,
    marginBottom: 20,
    shadowColor: '#000',
    shadowOffset: { width: 0, height: 2 },
    shadowOpacity: 0.1,
    shadowRadius: 4,
    elevation: 2,
  },
  text: {
    fontSize: 16,
    marginBottom: 8,
    color: '#333',
  },
  permissionGranted: {
    color: 'green',
    fontWeight: 'bold',
  },
  permissionDenied: {
    color: 'red',
    fontWeight: 'bold',
  },
  buttonContainer: {
    marginBottom: 20,
  },
  spacer: {
    height: 16,
  },
  permissionWarning: {
    backgroundColor: '#ffebee',
    padding: 16,
    borderRadius: 8,
    marginBottom: 20,
    borderLeftWidth: 4,
    borderLeftColor: '#f44336',
  },
  warningText: {
    color: '#d32f2f',
    fontWeight: 'bold',
  },
  instructions: {
    backgroundColor: '#e3f2fd',
    padding: 16,
    borderRadius: 8,
    borderLeftWidth: 4,
    borderLeftColor: '#2196f3',
  },
  instructionText: {
    fontSize: 14,
    color: '#1976d2',
    marginBottom: 8,
  },
});