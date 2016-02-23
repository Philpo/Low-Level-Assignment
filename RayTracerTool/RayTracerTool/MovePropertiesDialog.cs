using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace RayTracerTool {
  public partial class MovePropertiesDialog : Form {
    private String function;
    private float amount;

    public MovePropertiesDialog(Move move, int numSpheres) {
      InitializeComponent();
      
      comboBox1.Text = move.getFunction();
      numericUpDown2.Text = move.getAmount().ToString();
    }

    private void button1_Click(object sender, EventArgs e) {
      function = comboBox1.SelectedItem.ToString();
      amount = float.Parse(numericUpDown2.Text);

      this.Close();
    }

    private void button2_Click(object sender, EventArgs e) {
      this.Close();
    }

    public String getFunction() {
      return function;
    }

    public float getAmount() {
      return amount;
    }
  }
}
