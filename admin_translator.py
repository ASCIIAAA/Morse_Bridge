import serial
import time
from googletrans import Translator

# --- CONFIGURATION ---
# Replace 'COM3' with your Admin Arduino Port (Check Device Manager or Arduino IDE)
# On Mac/Linux, it will look like '/dev/ttyUSB0' or '/dev/cu.usbmodem...'
SERIAL_PORT = 'COM13' 
BAUD_RATE = 9600

def start_translator():
    translator = Translator()
    
    print(f"--- CONNECTING TO ADMIN UNIT ON {SERIAL_PORT} ---")
    
    try:
        ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
        time.sleep(2) # Wait for connection to stabilize
        print("--- CONNECTED! WAITING FOR MESSAGES... ---")
        print("(Press Ctrl+C to Exit)")
        print("-" * 50)

        while True:
            # 1. Read line from Arduino
            if ser.in_waiting > 0:
                line = ser.readline().decode('utf-8', errors='ignore').strip()
                
                # 2. Filter for actual messages (Your logs look like: "[SPY] > MESSAGE")
                if "[SPY] >" in line:
                    # Extract just the message part
                    # Split at ">", take the second part, and strip whitespace
                    parts = line.split(">")
                    if len(parts) > 1:
                        english_msg = parts[1].strip()
                        
                        # 3. Translate to Hindi
                        try:
                            translation = translator.translate(english_msg, src='en', dest='hi')
                            hindi_msg = translation.text
                            
                            # 4. Display Result
                            print(f"\n📨 INCOMING MSG: {english_msg}")
                            print(f"🇮🇳 HINDI TRANSLATION: {hindi_msg}")
                            print("-" * 50)
                            
                        except Exception as e:
                            print(f"Translation Error: {e}")
                
                # Optional: Print other logs (SYSTEM/ADMIN) just so you see them
                elif "[ADMIN]" in line or "[SYSTEM]" in line:
                    print(f"[LOG] {line}")

    except serial.SerialException:
        print(f"ERROR: Could not open port {SERIAL_PORT}. Is the Arduino connected?")
    except KeyboardInterrupt:
        print("\n--- TRANSLATOR STOPPED ---")
        if 'ser' in locals() and ser.is_open:
            ser.close()

if __name__ == "__main__":
    start_translator()