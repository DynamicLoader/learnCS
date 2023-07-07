class junkTimer {
  junkTimer() {
    var startTime = 0;
      var endTime = 0;
      var elapsed = 0;
  }

  start() {
      this.startTime = Date.now();
      console.log(`Timer started in ts = ${this.startTime.toString}`)
  }

  stop() {
      this.endTime = Date.now();
      elapsed = this.endTime - this.startTime;
      console.log('Timer ended in ' + this.endTime.toString)
  }
}
