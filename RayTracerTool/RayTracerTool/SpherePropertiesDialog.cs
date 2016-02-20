using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.Windows.Media;

namespace RayTracerTool {
  public partial class SpherePropertiesDialog : Form {
    private float x, y, z, radius, reflection, transparency;
    private System.Windows.Media.Color surface, emission;

    public SpherePropertiesDialog(Sphere sphere) {
      InitializeComponent();

      numericUpDown1.Text = sphere.getX().ToString();
      numericUpDown2.Text = sphere.getY().ToString();
      numericUpDown3.Text = sphere.getZ().ToString();
      numericUpDown4.Text = sphere.getRadius().ToString();
      numericUpDown5.Text = sphere.getReflection().ToString();
      numericUpDown6.Text = sphere.getTransparency().ToString();
      surface = sphere.getSurface();
      emission = sphere.getEmission();
    }

    private void button1_Click(object sender, EventArgs e) {
      ColorDialog dialog = new ColorDialog();
      dialog.Color = System.Drawing.Color.FromArgb(surface.A, surface.R, surface.G, surface.B);
      dialog.FullOpen = true;
      DialogResult result = dialog.ShowDialog();

      if (result == DialogResult.OK) {
        surface = System.Windows.Media.Color.FromArgb(dialog.Color.A, dialog.Color.R, dialog.Color.G, dialog.Color.B);
      }
    }

    private void button2_Click(object sender, EventArgs e) {
      ColorDialog dialog = new ColorDialog();
      dialog.Color = System.Drawing.Color.FromArgb(emission.A, emission.R, emission.G, emission.B);
      dialog.FullOpen = true;
      DialogResult result = dialog.ShowDialog();

      if (result == DialogResult.OK) {
        emission = System.Windows.Media.Color.FromArgb(dialog.Color.A, dialog.Color.R, dialog.Color.G, dialog.Color.B);
      }
    }

    private void button3_Click(object sender, EventArgs e) {
      x = float.Parse(numericUpDown1.Text);
      y = float.Parse(numericUpDown2.Text);
      z = float.Parse(numericUpDown3.Text);
      radius = float.Parse(numericUpDown4.Text);
      reflection = float.Parse(numericUpDown5.Text);
      transparency = float.Parse(numericUpDown6.Text);

      this.Close();
    }

    private void button4_Click(object sender, EventArgs e) {
      this.Close();
    }

    public float getX() {
      return x;
    }

    public float getY() {
      return y;
    }

    public float getZ() {
      return z;
    }

    public float getRadius() {
      return radius;
    }

    public float getReflection() {
      return reflection;
    }

    public float getTransparency() {
      return transparency;
    }

    public System.Windows.Media.Color getSurface() {
      return surface;
    }

    public System.Windows.Media.Color getEmission() {
      return emission;
    }
  }
}
