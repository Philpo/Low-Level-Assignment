namespace RayTracerTool {
  partial class SpheresTabPage {
    /// <summary> 
    /// Required designer variable.
    /// </summary>
    private System.ComponentModel.IContainer components = null;

    /// <summary> 
    /// Clean up any resources being used.
    /// </summary>
    /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
    protected override void Dispose(bool disposing) {
      if (disposing && (components != null)) {
        components.Dispose();
      }
      base.Dispose(disposing);
    }

    #region Component Designer generated code

    /// <summary> 
    /// Required method for Designer support - do not modify 
    /// the contents of this method with the code editor.
    /// </summary>
    private void InitializeComponent() {
      this.treeView1 = new System.Windows.Forms.TreeView();
      this.button1 = new System.Windows.Forms.Button();
      this.button2 = new System.Windows.Forms.Button();
      this.SuspendLayout();
      // 
      // treeView1
      // 
      this.treeView1.Location = new System.Drawing.Point(4, 4);
      this.treeView1.Name = "treeView1";
      this.treeView1.Size = new System.Drawing.Size(375, 310);
      this.treeView1.TabIndex = 0;
      // 
      // button1
      // 
      this.button1.Location = new System.Drawing.Point(4, 320);
      this.button1.Name = "button1";
      this.button1.Size = new System.Drawing.Size(75, 23);
      this.button1.TabIndex = 1;
      this.button1.Text = "Add Sphere";
      this.button1.UseVisualStyleBackColor = true;
      // 
      // button2
      // 
      this.button2.Location = new System.Drawing.Point(86, 320);
      this.button2.Name = "button2";
      this.button2.Size = new System.Drawing.Size(101, 23);
      this.button2.TabIndex = 2;
      this.button2.Text = "Remove Sphere";
      this.button2.UseVisualStyleBackColor = true;
      // 
      // SpheresTabPage
      // 
      this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
      this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
      this.Controls.Add(this.button2);
      this.Controls.Add(this.button1);
      this.Controls.Add(this.treeView1);
      this.Name = "SpheresTabPage";
      this.Size = new System.Drawing.Size(382, 346);
      this.ResumeLayout(false);

    }

    #endregion

    public System.Windows.Forms.TreeView treeView1;
    public System.Windows.Forms.Button button1;
    public System.Windows.Forms.Button button2;
  }
}
