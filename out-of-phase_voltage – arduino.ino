/*
 * Измеритель мостового (противофазного) сигнала ATtiny13 на Arduino Nano
 * Подключение (3 провода напрямую):
 *   GND (ATtiny13) -> GND (Arduino Nano)
 *   PB0 (ATtiny13) -> A0  (Arduino Nano)
 *   PB1 (ATtiny13) -> A1  (Arduino Nano)
 */

// --- Настройки пинов и параметров ---
const uint8_t PIN_PB0 = A0;            // Первый контакт пьезоизлучателя
const uint8_t PIN_PB1 = A1;            // Второй контакт пьезоизлучателя
const float V_REF = 4.85;              // Опорное / питающее напряжение схемы (4.85 В)
const unsigned long WINDOW_MS = 500;   // Интервал обновления пиковых значений (0.5 сек)

// --- Глобальные переменные для замеров ---
float v0 = 0.0;                        // Напряжение на PB0 относительно GND
float v1 = 0.0;                        // Напряжение на PB1 относительно GND
float vDiff = 0.0;                     // Реальная разность потенциалов на пьезике (PB0 - PB1)
float minDiff = 0.0;                   // Минимальный пик за окно замера
float maxDiff = 0.0;                   // Максимальный пик за окно замера
float vPeakToPeak = 0.0;               // Итоговый размах от пика до пика (Vpp)
unsigned long lastResetTime = 0;       // Таймер сброса окна

void setup() {
  initHardware();                      // Вызов процедуры инициализации
}

void loop() {
  readPinVoltages();                   // 1. Считываем напряжения на обоих пинах
  calculateBridgeAmplitude();          // 2. Считаем разность и полный размах Vpp
  sendToPlotter();                     // 3. Выводим графики в Serial Plotter
  delayMicroseconds(500);              // Небольшая задержка для стабильности графика
}

// =========================================================================
// ПРОЦЕДУРА 1: Настройка порта и входов АЦП
// =========================================================================
void initHardware() {
  Serial.begin(115200);                // Высокая скорость нужна для быстрой отрисовки волны
  pinMode(PIN_PB0, INPUT);
  pinMode(PIN_PB1, INPUT);
}

// =========================================================================
// ПРОЦЕДУРА 2: Чтение напряжений с пинов PB0 и PB1 относительно общей земли
// =========================================================================
void readPinVoltages() {
  int raw0 = analogRead(PIN_PB0);      // Читаем АЦП на A0 (число от 0 до 1023)
  int raw1 = analogRead(PIN_PB1);      // Читаем АЦП на A1 (число от 0 до 1023)

  v0 = (raw0 * V_REF) / 1023.0;        // Переводим попугаи АЦП в Вольты для PB0
  v1 = (raw1 * V_REF) / 1023.0;        // Переводим попугаи АЦП в Вольты для PB1
}

// =========================================================================
// ПРОЦЕДУРА 3: Расчёт дифференциального напряжения и размаха (Peak-to-Peak)
// =========================================================================
void calculateBridgeAmplitude() {
  // Реальное напряжение, которое "видит" нагрузка между выводами PB0 и PB1:
  // Когда PB0 = +U, а PB1 = 0В -> vDiff = +U
  // Когда PB0 = 0В, а PB1 = +U -> vDiff = -U
  vDiff = v0 - v1;

  // Запоминаем верхний и нижний пики сигнала за текущие 0.5 секунды
  if (vDiff < minDiff) minDiff = vDiff;
  if (vDiff > maxDiff) maxDiff = vDiff;

  // Полный размах от самого нижнего до самого верхнего пика
  vPeakToPeak = maxDiff - minDiff;

  // Каждые 500 мс сбрасываем окно пиков, чтобы график реагировал на тишину
  if (millis() - lastResetTime >= WINDOW_MS) {
    minDiff = vDiff;
    maxDiff = vDiff;
    lastResetTime = millis();
  }
}

// =========================================================================
// ПРОЦЕДУРА 4: Отправка данных в Serial Plotter Arduino IDE
// =========================================================================
void sendToPlotter() {
  Serial.print("PB0_V:");
  Serial.print(v0, 2);
  Serial.print("\t");

  Serial.print("PB1_V:");
  Serial.print(v1, 2);
  Serial.print("\t");

  Serial.print("Piezo_Diff_V:");
  Serial.print(vDiff, 2);
  Serial.print("\t");

  Serial.print("Razmah_Vpp:");
  Serial.println(vPeakToPeak, 2);
}
