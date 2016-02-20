using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace RayTracerTool {
  public class Sphere {
    private float x, y, z, radius, reflection, transparency;
    private System.Windows.Media.Color surface, emission;

    public Sphere() {
      surface = System.Windows.Media.Color.FromScRgb(1.0f, 0.0f, 0.0f, 0.0f);
      emission = System.Windows.Media.Color.FromScRgb(1.0f, 0.0f, 0.0f, 0.0f);
    }

    public Sphere(float x, float y, float z, float radius, System.Windows.Media.Color surface, System.Windows.Media.Color emission, float reflection = 0.0f, float transparency = 0.0f) {
      this.x = x;
      this.y = y;
      this.z = z;
      this.radius = radius;
      this.surface = surface;
      this.emission = emission;
      this.reflection = reflection;
      this.transparency = transparency;
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

    public void setX(float x) {
      this.x = x;
    }

    public void setY(float y) {
      this.y = y;
    }

    public void setZ(float z) {
      this.z = z;
    }

    public void setRadius(float radius) {
      this.radius = radius;
    }

    public void setReflection(float reflection) {
      this.reflection = reflection;
    }

    public void setTransparency(float transparency) {
      this.transparency = transparency;
    }

    public void setSurface(System.Windows.Media.Color surface) {
      this.surface = surface;
    }

    public void setEmission(System.Windows.Media.Color emission) {
      this.emission = emission;
    }
  }
}
