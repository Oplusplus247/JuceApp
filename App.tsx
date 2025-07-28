import React, { useEffect, useState } from 'react';
import {SafeAreaView, ScrollView, StatusBar, Text, View, Button, Alert, StyleSheet, Switch,} from 'react-native';
import Slider from '@react-native-community/slider';
import { NativeModules } from 'react-native';

const App = () => {
  const [moduleStatus, setModuleStatus] = useState('Checking...');
  const [juceStatus, setJuceStatus] = useState('Not initialized');
  const [lastResult, setLastResult] = useState('');
  const [audioInfo, setAudioInfo] = useState<any>(null);
  
  const [toneFrequency, setToneFrequency] = useState(440);
  const [toneVolume, setToneVolume] = useState(0.5
  );
  const [isTonePlaying, setIsTonePlaying] = useState(false);
  const [toneStatus, setToneStatus] = useState<any>(null);

  useEffect(() => {
    checkModuleStatus();
  }, []);

  const checkModuleStatus = () => {
    try {
      const { AudioModule } = NativeModules;
      if (AudioModule) {
        const methods = Object.keys(AudioModule);
        setModuleStatus(`✅ AudioModule found with ${methods.length} methods`);
        console.log('AudioModule methods:', methods);
      } else {
        setModuleStatus('AudioModule not found');
      }
    } catch (error) {
      setModuleStatus(`Error: ${error}`);
    }
  };

  const testBasicBridge = async () => {
    try {
      const { AudioModule } = NativeModules;
      if (!AudioModule) {
        Alert.alert('Error', 'AudioModule not available');
        return;
      }

      const version = await AudioModule.getVersion();
      const echo = await AudioModule.testBridge('Hello JUCE!');
      
      setLastResult(`✅ Basic Bridge Working\nVersion: ${version}\nEcho: ${echo}`);
      Alert.alert('Basic Bridge Success! 🎉', `Version: ${version}\nEcho: ${echo}`);
    } catch (error) {
      console.error('Basic bridge error:', error);
      Alert.alert('Error', `Basic bridge failed: ${error.message}`);
    }
  };

  const testJuceJNI = async () => {
    try {
      const { AudioModule } = NativeModules;
      if (!AudioModule) {
        Alert.alert('Error', 'AudioModule not available');
        return;
      }

      const jniMessage = await AudioModule.getStringFromJNI();
      setLastResult(`✅ JUCE JNI Working\nMessage: ${jniMessage}`);
      Alert.alert('JUCE JNI Success! 🎵', `Message: ${jniMessage}`);
    } catch (error) {
      console.error('JUCE JNI error:', error);
      Alert.alert('Error', `JUCE JNI failed: ${error.message}`);
    }
  };

  const initializeJUCE = async () => {
    try {
      const { AudioModule } = NativeModules;
      if (!AudioModule) {
        Alert.alert('Error', 'AudioModule not available');
        return;
      }

      setJuceStatus('Initializing...');
      const success = await AudioModule.initializeJUCE();
      
      if (success) {
        setJuceStatus('✅ JUCE Initialized');
        
        const version = await AudioModule.getJUCEVersion();
        setLastResult(`✅ JUCE Initialized\nVersion: ${version}`);
        
        Alert.alert('JUCE Initialized! 🎵', `JUCE Version: ${version}\n\nYou can now use the tone generator!`);
        
        updateToneStatus();
      } else {
        setJuceStatus('❌ JUCE Failed to Initialize');
        Alert.alert('Error', 'Failed to initialize JUCE');
      }
    } catch (error) {
      console.error('JUCE init error:', error);
      setJuceStatus('❌ JUCE Init Error');
      Alert.alert('Error', `JUCE initialization failed: ${error.message}`);
    }
  };

  const getAudioInfo = async () => {
    try {
      const { AudioModule } = NativeModules;
      if (!AudioModule) {
        Alert.alert('Error', 'AudioModule not available');
        return;
      }

      const info = await AudioModule.getAudioInfo();
      setAudioInfo(info);
      
      const infoText = `
        Device: ${info.deviceName}
        Sample Rate: ${info.sampleRate} Hz
        Output Channels: ${info.outputChannels}
        Buffer Size: ${info.bufferSize} samples`;
      
      setLastResult(`✅ Audio Info Retrieved${infoText}`);
      Alert.alert('Audio Info 🔊', infoText.trim());
    } catch (error) {
      console.error('Audio info error:', error);
      Alert.alert('Error', `Failed to get audio info: ${error.message}`);
    }
  };

  const shutdownJUCE = async () => {
    try {
      const { AudioModule } = NativeModules;
      if (!AudioModule) {
        Alert.alert('Error', 'AudioModule not available');
        return;
      }

      if (isTonePlaying) {
        await stopTone();
      }

      await AudioModule.shutdownJUCE();
      setJuceStatus('🔴 JUCE Shut Down');
      setAudioInfo(null);
      setToneStatus(null);
      setLastResult('✅ JUCE shut down successfully');
      Alert.alert('JUCE Shutdown', 'JUCE audio engine shut down successfully');
    } catch (error) {
      console.error('JUCE shutdown error:', error);
      Alert.alert('Error', `JUCE shutdown failed: ${error.message}`);
    }
  };

  const updateToneStatus = async () => {
    try {
      const { AudioModule } = NativeModules;
      if (!AudioModule) return;

      const status = await AudioModule.getToneGeneratorStatus();
      setToneStatus(status);
      setIsTonePlaying(status.isPlaying);
      setToneFrequency(status.frequency);
      setToneVolume(status.volume);
    } catch (error) {
      console.error('Error getting tone status:', error);
    }
  };

  const toggleTone = async () => {
    try {
      const { AudioModule } = NativeModules;
      if (!AudioModule) {
        Alert.alert('Error', 'AudioModule not available');
        return;
      }

      if (juceStatus !== '✅ JUCE Initialized') {
        Alert.alert('Initialize JUCE First', 'Please initialize JUCE before using the tone generator');
        return;
      }

      if (isTonePlaying) {
        await stopTone();
      } else {
        await startTone();
      }
    } catch (error) {
      console.error('Toggle tone error:', error);
      Alert.alert('Error', `Failed to toggle tone: ${error.message}`);
    }
  };

  const startTone = async () => {
    try {
      const { AudioModule } = NativeModules;
      await AudioModule.startToneGenerator();
      setIsTonePlaying(true);
      setLastResult(`🎵 Tone Started!\nFrequency: ${toneFrequency} Hz\nVolume: ${Math.round(toneVolume * 100)}%`);
      updateToneStatus();
    } catch (error) {
      console.error('Start tone error:', error);
      Alert.alert('Error', `Failed to start tone: ${error.message}`);
    }
  };

  const stopTone = async () => {
    try {
      const { AudioModule } = NativeModules;
      await AudioModule.stopToneGenerator();
      setIsTonePlaying(false);
      setLastResult('🔇 Tone Stopped');
      updateToneStatus();
    } catch (error) {
      console.error('Stop tone error:', error);
      Alert.alert('Error', `Failed to stop tone: ${error.message}`);
    }
  };

  const updateFrequency = async (frequency: number) => {
    try {
      const { AudioModule } = NativeModules;
      if (!AudioModule) return;

      await AudioModule.setToneGeneratorFrequency(frequency);
      setToneFrequency(frequency);
      updateToneStatus();
    } catch (error) {
      console.error('Update frequency error:', error);
    }
  };

  const updateVolume = async (volume: number) => {
    try {
      const { AudioModule } = NativeModules;
      if (!AudioModule) return;

      await AudioModule.setToneGeneratorVolume(volume);
      setToneVolume(volume);
      updateToneStatus();
    } catch (error) {
      console.error('Update volume error:', error);
    }
  };

  const runFullJuceTest = async () => {
    try {
      const { AudioModule } = NativeModules;
      if (!AudioModule) {
        Alert.alert('Error', 'AudioModule not available');
        return;
      }

      setLastResult('🧪 Running full JUCE test...');
      
      const jniMessage = await AudioModule.getStringFromJNI();
      console.log('✅ JNI Test:', jniMessage);
      
      const initSuccess = await AudioModule.initializeJUCE();
      if (!initSuccess) {
        throw new Error('JUCE initialization failed');
      }
      console.log('✅ JUCE Initialized');
      
      const version = await AudioModule.getJUCEVersion();
      console.log('✅ JUCE Version:', version);
      
      const info = await AudioModule.getAudioInfo();
      console.log('✅ Audio Info:', info);
      
      await AudioModule.setToneGeneratorFrequency(880);
      await AudioModule.setToneGeneratorVolume(0.1);
      await AudioModule.startToneGenerator();
      
      setTimeout(async () => {
        await AudioModule.stopToneGenerator();
        console.log('✅ Tone Generator Test Completed');
      }, 1000);
      
      setJuceStatus('✅ JUCE Fully Tested');
      setAudioInfo(info);
      updateToneStatus();
      
      setLastResult(`🎉 Full JUCE Test Passed!
        Version: ${version}
        Device: ${info.deviceName}
        Sample Rate: ${info.sampleRate} Hz
        Channels: ${info.outputChannels}
        Buffer: ${info.bufferSize} samples
        🎵 Tone Generator: Working!`);
      
      Alert.alert(
        'Full JUCE Test Passed! 🎉',
        `✅ JNI Communication
          ✅ JUCE Initialization
          ✅ Version: ${version}
          ✅ Audio Device: ${info.deviceName}
          ✅ Sample Rate: ${info.sampleRate} Hz
          ✅ Tone Generator: Working!
          Your JUCE React Native bridge is fully functional!`
      );
      
    } catch (error) {
      console.error('Full JUCE test error:', error);
      setJuceStatus('❌ JUCE Test Failed');
      Alert.alert('Test Failed', `Full JUCE test failed: ${error.message}`);
    }
  };

  return (
    <SafeAreaView style={styles.container}>
      <StatusBar barStyle="dark-content" />
      <ScrollView contentInsetAdjustmentBehavior="automatic">
        <View style={styles.body}>
          
          <Text style={styles.title}>🎵 JUCE Audio Bridge</Text>
          <Text style={styles.subtitle}>React Native ↔ JUCE ↔ Android</Text>
          
          <View style={styles.statusContainer}>
            <Text style={styles.statusLabel}>Module Status:</Text>
            <Text style={styles.statusText}>{moduleStatus}</Text>
            <Text style={styles.statusLabel}>JUCE Status:</Text>
            <Text style={styles.statusText}>{juceStatus}</Text>
          </View>

          <View style={styles.toneGeneratorContainer}>
            <Text style={styles.toneGeneratorTitle}>🎵 Tone Generator</Text>
            
            <View style={styles.toneControlsContainer}>
              <View style={styles.toneControl}>
                <Text style={styles.toneControlLabel}>
                  Frequency: {Math.round(toneFrequency)} Hz
                </Text>
                <Slider
                  style={styles.slider}
                  minimumValue={20}
                  maximumValue={2000}
                  value={toneFrequency}
                  onValueChange={updateFrequency}
                  minimumTrackTintColor="#FF9500"
                  maximumTrackTintColor="#E0E0E0"
                  thumbTintColor="#FF9500"
                />
              </View>
              
              <View style={styles.toneControl}>
                <Text style={styles.toneControlLabel}>
                  Volume: {Math.round(toneVolume * 100)}%
                </Text>
                <Slider
                  style={styles.slider}
                  minimumValue={0}
                  maximumValue={1}
                  value={toneVolume}
                  onValueChange={updateVolume}
                  minimumTrackTintColor="#34C759"
                  maximumTrackTintColor="#E0E0E0"
                  thumbTintColor="#34C759"
                />
              </View>
              
              <View style={styles.tonePlayContainer}>
                <Text style={styles.toneControlLabel}>
                  Status: {isTonePlaying ? '🔊 Playing' : '🔇 Stopped'}
                </Text>
                <View style={styles.toneButtonContainer}>
                  <Button
                    title={isTonePlaying ? "🔇 Stop Tone" : "🔊 Play Tone"}
                    onPress={toggleTone}
                    color={isTonePlaying ? "#FF3B30" : "#34C759"}
                  />
                </View>
              </View>
            </View>
          </View>

          {audioInfo && (
            <View style={styles.audioInfoContainer}>
              <Text style={styles.audioInfoLabel}>Audio Device Info:</Text>
              <Text style={styles.audioInfoText}>
                🔊 {audioInfo.deviceName}{'\n'}
                📊 {audioInfo.sampleRate} Hz, {audioInfo.outputChannels} channels{'\n'}
                🔧 Buffer: {audioInfo.bufferSize} samples
              </Text>
            </View>
          )}

          {lastResult ? (
            <View style={styles.resultContainer}>
              <Text style={styles.resultLabel}>Last Result:</Text>
              <Text style={styles.resultText}>{lastResult}</Text>
            </View>
          ) : null}

          <View style={styles.buttonContainer}>
            <Button 
              title="🔍 Check Module Status" 
              onPress={checkModuleStatus}
              color="#007AFF"
            />
            
            <View style={styles.buttonSpacer} />
            
            <Button 
              title="🌉 Test Basic Bridge" 
              onPress={testBasicBridge}
              color="#34C759"
            />
            
            <View style={styles.buttonSpacer} />
            
            <Button 
              title="🎵 Test JUCE JNI" 
              onPress={testJuceJNI}
              color="#FF9500"
            />
            
            <View style={styles.buttonSpacer} />
            
            <Button 
              title="🚀 Initialize JUCE" 
              onPress={initializeJUCE}
              color="#AF52DE"
            />
            
            <View style={styles.buttonSpacer} />
            
            <Button 
              title="🔊 Get Audio Info" 
              onPress={getAudioInfo}
              color="#FF2D92"
            />
            
            <View style={styles.buttonSpacer} />
            
            <Button 
              title="🧪 Run Full JUCE Test" 
              onPress={runFullJuceTest}
              color="#00C7BE"
            />
            
            <View style={styles.buttonSpacer} />
            
            <Button 
              title="🔴 Shutdown JUCE" 
              onPress={shutdownJUCE}
              color="#FF3B30"
            />
          </View>

        </View>
      </ScrollView>
    </SafeAreaView>
  );
};

const styles = StyleSheet.create({
  container: {
    flex: 1,
    backgroundColor: '#F2F2F7',
  },
  body: {
    padding: 20,
    alignItems: 'center',
    justifyContent: 'center',
    minHeight: '100%',
  },
  title: {
    fontSize: 28,
    fontWeight: 'bold',
    marginBottom: 5,
    textAlign: 'center',
    color: '#1D1D1F',
  },
  subtitle: {
    fontSize: 16,
    color: '#8E8E93',
    marginBottom: 30,
    textAlign: 'center',
  },
  statusContainer: {
    backgroundColor: '#FFFFFF',
    padding: 15,
    borderRadius: 10,
    marginBottom: 20,
    width: '100%',
    shadowColor: '#000',
    shadowOffset: { width: 0, height: 1 },
    shadowOpacity: 0.1,
    shadowRadius: 3,
    elevation: 2,
  },
  statusLabel: {
    fontSize: 14,
    fontWeight: '600',
    color: '#8E8E93',
    marginBottom: 5,
    marginTop: 5,
  },
  statusText: {
    fontSize: 16,
    color: '#1D1D1F',
    fontFamily: 'monospace',
  },
  toneGeneratorContainer: {
    backgroundColor: '#FFF5E6',
    padding: 20,
    borderRadius: 10,
    marginBottom: 20,
    width: '100%',
    borderLeftWidth: 4,
    borderLeftColor: '#FF9500',
  },
  toneGeneratorTitle: {
    fontSize: 18,
    fontWeight: 'bold',
    color: '#B8560A',
    marginBottom: 15,
    textAlign: 'center',
  },
  toneControlsContainer: {
    width: '100%',
  },
  toneControl: {
    marginBottom: 20,
  },
  toneControlLabel: {
    fontSize: 16,
    fontWeight: '600',
    color: '#B8560A',
    marginBottom: 8,
  },
  slider: {
    width: '100%',
    height: 40,
  },
  tonePlayContainer: {
    alignItems: 'center',
  },
  toneButtonContainer: {
    marginTop: 10,
    width: '60%',
  },
  audioInfoContainer: {
    backgroundColor: '#E8F4FD',
    padding: 15,
    borderRadius: 10,
    marginBottom: 20,
    width: '100%',
    borderLeftWidth: 4,
    borderLeftColor: '#007AFF',
  },
  audioInfoLabel: {
    fontSize: 14,
    fontWeight: '600',
    color: '#0056CC',
    marginBottom: 5,
  },
  audioInfoText: {
    fontSize: 14,
    color: '#0056CC',
    fontFamily: 'monospace',
  },
  resultContainer: {
    backgroundColor: '#E8F5E8',
    padding: 15,
    borderRadius: 10,
    marginBottom: 20,
    width: '100%',
    borderLeftWidth: 4,
    borderLeftColor: '#34C759',
  },
  resultLabel: {
    fontSize: 14,
    fontWeight: '600',
    color: '#1D6B20',
    marginBottom: 5,
  },
  resultText: {
    fontSize: 14,
    color: '#1D6B20',
    fontFamily: 'monospace',
  },
  buttonContainer: {
    width: '100%',
  },
  buttonSpacer: {
    height: 12,
  },
});

export default App;