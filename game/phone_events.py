from dataclasses import dataclass


@dataclass
class PhoneEvents:
    offhook: bool = False
    hungup: bool = False
 
    press_1: bool = False   
    press_2: bool = False
    press_3: bool = False
    press_4: bool = False
    press_5: bool = False
    press_6: bool = False
    press_7: bool = False
    press_8: bool = False
    press_9: bool = False
    press_0: bool = False
    press_star: bool = False
    press_pound: bool = False

    def any_set(self):
        return any([
            self.offhook,
            self.hungup,
            self.press_1,
            self.press_2,
            self.press_3,
            self.press_4,
            self.press_5,
            self.press_6,
            self.press_7,
            self.press_8,
            self.press_9,
            self.press_0,
            self.press_star,
            self.press_pound,
        ])