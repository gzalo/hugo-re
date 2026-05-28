export class PhoneEvents {
  offhook = false;
  hungup = false;
  press_0 = false;
  press_1 = false;
  press_2 = false;
  press_3 = false;
  press_4 = false;
  press_5 = false;
  press_6 = false;
  press_7 = false;
  press_8 = false;
  press_9 = false;
  press_star = false;
  press_pound = false;

  anySet(): boolean {
    return (
      this.offhook || this.hungup ||
      this.press_0 || this.press_1 || this.press_2 || this.press_3 ||
      this.press_4 || this.press_5 || this.press_6 || this.press_7 ||
      this.press_8 || this.press_9 || this.press_star || this.press_pound
    );
  }
}
