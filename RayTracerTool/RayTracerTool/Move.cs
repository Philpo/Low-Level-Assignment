using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace RayTracerTool {
  public class Move {
    private int target;
    private String function;
    private float amount;

    public Move() { }

    public Move(int target, String function, float amount) {
      this.target = target;
      this.function = function;
      this.amount = amount;
    }

    public int getTarget() {
      return target;
    }

    public String getFunction() {
      return function;
    }

    public float getAmount() {
      return amount;
    }

    public void setTarget(int target) {
      this.target = target;
    }

    public void setFunction(String function) {
      this.function = function;
    }

    public void setAmount(float amount) {
      this.amount = amount;
    }
  }
}
