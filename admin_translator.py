import serial
import time
from deep_translator import GoogleTranslator

# --- CONFIGURATION ---
# Replace 'COM3' with your Admin Arduino Port
SERIAL_PORT = 'COM13' 
BAUD_RATE = 9600

def start_translator():
    # Initialize Translators
    translator_hi = GoogleTranslator(source='auto', target='hi') # Hindi
    translator_mr = GoogleTranslator(source='auto', target='mr') # Marathi
    
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
                
                # 2. Filter for actual messages
                if "[SPY] >" in line:
                    parts = line.split(">")
                    if len(parts) > 1:
                        english_msg = parts[1].strip()
                        
                        # 3. Translate
                        try:
                            hindi_msg = translator_hi.translate(english_msg)
                            marathi_msg = translator_mr.translate(english_msg)
                            
                            # 4. Display Result
                            print(f"\nINCOMING MSG: {english_msg}")
                            print(f"HINDI:   {hindi_msg}")
                            print(f"MARATHI: {marathi_msg}")
                            print("-" * 50)
                            
                        except Exception as e:
                            print(f"Translation Error: {e}")
                
                # Optional: Print other logs
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