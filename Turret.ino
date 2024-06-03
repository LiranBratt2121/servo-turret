#include <Pixy2.h>
#include <Servo.h>

const int CENTER_X = 648; // Half of Pixy2 resolution width (1296/2)
const int CENTER_Y = 488; // Half of Pixy2 resolution height (976/2)

const int SERVO_MIN = 0;
const int SERVO_MAX = 180;

const int SERVO_X_PIN = 9;
const int SERVO_Y_PIN = 10;

Pixy2 pixy;

class PController {
  public:
    float m_kp;
    bool m_is_x;

    PController(float kp, bool is_x) : m_kp(kp), m_is_x(is_x) {}

    float calculate(int position) {
      float setpoint = m_is_x ? CENTER_X : CENTER_Y;
      return m_kp * (setpoint - position);      
    }
};

class Turret {
  private:
    Servo m_servo_x, m_servo_y;
    PController m_controller_x, m_controller_y;

  public:
    Turret() : m_controller_x(0.1, true), m_controller_y(0.1, false) {
      m_servo_x.attach(SERVO_X_PIN);
      m_servo_y.attach(SERVO_Y_PIN);
    }

    void begin() {
      pixy.init();
      pixy.changeProg("color_connected_components");
    }

    void update() {
      pixy.ccc.getBlocks();
      
      if (pixy.ccc.numBlocks) {
        int x_pos = pixy.ccc.blocks[0].m_x;
        int y_pos = pixy.ccc.blocks[0].m_y;

        // Calculate control signals
        float control_signal_x = m_controller_x.calculate(x_pos);
        float control_signal_y = m_controller_y.calculate(y_pos);

        // Update servo positions
        int new_pos_x = constrain(m_servo_x.read() + control_signal_x, SERVO_MIN, SERVO_MAX);
        int new_pos_y = constrain(m_servo_y.read() + control_signal_y, SERVO_MIN, SERVO_MAX);

        m_servo_x.write(new_pos_x);
        m_servo_y.write(new_pos_y);
      }
    }
};

Turret turret;

void setup() {
  Serial.begin(115200);
  turret.begin();
}

void loop() {
  turret.update();
  delay(50);
}
