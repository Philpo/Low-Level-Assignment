namespace RayTracerTool {
  partial class Form1 {
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

    #region Windows Form Designer generated code

    /// <summary>
    /// Required method for Designer support - do not modify
    /// the contents of this method with the code editor.
    /// </summary>
    private void InitializeComponent() {
      this.menuStrip1 = new System.Windows.Forms.MenuStrip();
      this.fileToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
      this.openToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
      this.spheresFileToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
      this.movesFileToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
      this.tabControl1 = new System.Windows.Forms.TabControl();
      this.newToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
      this.spheresToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
      this.passToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
      this.saveToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
      this.menuStrip1.SuspendLayout();
      this.SuspendLayout();
      // 
      // menuStrip1
      // 
      this.menuStrip1.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.fileToolStripMenuItem});
      this.menuStrip1.Location = new System.Drawing.Point(0, 0);
      this.menuStrip1.Name = "menuStrip1";
      this.menuStrip1.Size = new System.Drawing.Size(883, 24);
      this.menuStrip1.TabIndex = 0;
      this.menuStrip1.Text = "menuStrip1";
      // 
      // fileToolStripMenuItem
      // 
      this.fileToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.newToolStripMenuItem,
            this.openToolStripMenuItem,
            this.saveToolStripMenuItem});
      this.fileToolStripMenuItem.Name = "fileToolStripMenuItem";
      this.fileToolStripMenuItem.Size = new System.Drawing.Size(37, 20);
      this.fileToolStripMenuItem.Text = "File";
      // 
      // openToolStripMenuItem
      // 
      this.openToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.spheresFileToolStripMenuItem,
            this.movesFileToolStripMenuItem});
      this.openToolStripMenuItem.Name = "openToolStripMenuItem";
      this.openToolStripMenuItem.Size = new System.Drawing.Size(152, 22);
      this.openToolStripMenuItem.Text = "Open";
      // 
      // spheresFileToolStripMenuItem
      // 
      this.spheresFileToolStripMenuItem.Name = "spheresFileToolStripMenuItem";
      this.spheresFileToolStripMenuItem.Size = new System.Drawing.Size(152, 22);
      this.spheresFileToolStripMenuItem.Text = "Spheres file";
      this.spheresFileToolStripMenuItem.Click += new System.EventHandler(this.spheresFileToolStripMenuItem_Click);
      // 
      // movesFileToolStripMenuItem
      // 
      this.movesFileToolStripMenuItem.Name = "movesFileToolStripMenuItem";
      this.movesFileToolStripMenuItem.Size = new System.Drawing.Size(152, 22);
      this.movesFileToolStripMenuItem.Text = "Moves file";
      this.movesFileToolStripMenuItem.Click += new System.EventHandler(this.movesFileToolStripMenuItem_Click);
      // 
      // tabControl1
      // 
      this.tabControl1.AllowDrop = true;
      this.tabControl1.Location = new System.Drawing.Point(0, 28);
      this.tabControl1.Name = "tabControl1";
      this.tabControl1.SelectedIndex = 0;
      this.tabControl1.Size = new System.Drawing.Size(883, 450);
      this.tabControl1.TabIndex = 1;
      // 
      // newToolStripMenuItem
      // 
      this.newToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.spheresToolStripMenuItem,
            this.passToolStripMenuItem});
      this.newToolStripMenuItem.Name = "newToolStripMenuItem";
      this.newToolStripMenuItem.Size = new System.Drawing.Size(152, 22);
      this.newToolStripMenuItem.Text = "New";
      // 
      // spheresToolStripMenuItem
      // 
      this.spheresToolStripMenuItem.Name = "spheresToolStripMenuItem";
      this.spheresToolStripMenuItem.Size = new System.Drawing.Size(152, 22);
      this.spheresToolStripMenuItem.Text = "Spheres";
      this.spheresToolStripMenuItem.Click += new System.EventHandler(this.spheresToolStripMenuItem_Click);
      // 
      // passToolStripMenuItem
      // 
      this.passToolStripMenuItem.Name = "passToolStripMenuItem";
      this.passToolStripMenuItem.Size = new System.Drawing.Size(152, 22);
      this.passToolStripMenuItem.Text = "Pass";
      this.passToolStripMenuItem.Click += new System.EventHandler(this.passToolStripMenuItem_Click);
      // 
      // saveToolStripMenuItem
      // 
      this.saveToolStripMenuItem.Name = "saveToolStripMenuItem";
      this.saveToolStripMenuItem.Size = new System.Drawing.Size(152, 22);
      this.saveToolStripMenuItem.Text = "Save";
      this.saveToolStripMenuItem.Click += new System.EventHandler(this.saveToolStripMenuItem_Click);
      // 
      // Form1
      // 
      this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
      this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
      this.ClientSize = new System.Drawing.Size(883, 478);
      this.Controls.Add(this.tabControl1);
      this.Controls.Add(this.menuStrip1);
      this.MainMenuStrip = this.menuStrip1;
      this.Name = "Form1";
      this.Text = "Form1";
      this.menuStrip1.ResumeLayout(false);
      this.menuStrip1.PerformLayout();
      this.ResumeLayout(false);
      this.PerformLayout();

    }

    #endregion

    private System.Windows.Forms.MenuStrip menuStrip1;
    private System.Windows.Forms.ToolStripMenuItem fileToolStripMenuItem;
    private System.Windows.Forms.ToolStripMenuItem openToolStripMenuItem;
    private System.Windows.Forms.ToolStripMenuItem spheresFileToolStripMenuItem;
    private System.Windows.Forms.ToolStripMenuItem movesFileToolStripMenuItem;
    private System.Windows.Forms.TabControl tabControl1;
    private System.Windows.Forms.ToolStripMenuItem newToolStripMenuItem;
    private System.Windows.Forms.ToolStripMenuItem spheresToolStripMenuItem;
    private System.Windows.Forms.ToolStripMenuItem passToolStripMenuItem;
    private System.Windows.Forms.ToolStripMenuItem saveToolStripMenuItem;
  }
}

