using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.IO;
using System.Xml;
using System.Windows.Media;

namespace RayTracerTool {
  public partial class Form1 : Form {
    private SpheresTabPage spheresLayout;
    private Dictionary<TabPage, PassTabPage> passLayouts = new Dictionary<TabPage,PassTabPage>();
    //private List<Sphere> spheres = new List<Sphere>();
    //private Dictionary<TabPage, int> numFrames = new Dictionary<TabPage, int>(), numThreads = new Dictionary<TabPage, int>();
    //private Dictionary<TabPage, List<Move>> passes = new Dictionary<TabPage,List<Move>>();
    private ContextMenu closeTabContextMenu = new ContextMenu();
    private Point dragStartPosition = Point.Empty;
    private int passCount = 1, sphereCount = 1;

    public Form1() {
      InitializeComponent();

      movesFileToolStripMenuItem.Enabled = false;
      passToolStripMenuItem.Enabled = false;
      saveToolStripMenuItem.Enabled = false;

      closeTabContextMenu.MenuItems.Add("Close Tab", new EventHandler(closeTab));
      tabControl1.MouseClick += new MouseEventHandler(tabControlMouseClick);
      tabControl1.MouseDown += new MouseEventHandler(tabControlMouseDown);
      tabControl1.MouseMove += new MouseEventHandler(tabControlMouseMove);
      tabControl1.DragOver += new DragEventHandler(tabControlDragOver);
      this.Resize += new EventHandler(resize);
      this.MinimumSize = new System.Drawing.Size(this.Width, this.Height);

      // no larger than screen size
      this.MaximumSize = new System.Drawing.Size((int) System.Windows.SystemParameters.PrimaryScreenWidth, (int) System.Windows.SystemParameters.PrimaryScreenHeight);

      this.AutoSize = true;
      this.AutoSizeMode = AutoSizeMode.GrowAndShrink;
    }

    private void resize(object sender, System.EventArgs e) {
      Control control = (Control) sender;

      Size newSize = new Size(control.Size.Width, control.Size.Height);
      control.Size = newSize;
      tabControl1.Size = new Size(control.Size.Width - 15, control.Size.Height - 50);
      foreach (TabPage tab in tabControl1.TabPages) {
        tab.Size = new Size(control.Size.Width - 15, control.Size.Height - 50);
      }
    }

    private void clearForm() {
      for (int i = tabControl1.Controls.Count - 1; i >= 0; i--) {
        tabControl1.Controls[i].Dispose();
      }
      //spheres.Clear();
      //numFrames.Clear();
      //numThreads.Clear();
      //passes.Clear();
      passCount = 1;
      sphereCount = 1;
    }

    private void spheresToolStripMenuItem_Click(object sender, EventArgs e) {
      clearForm();
      TabPage tab = new TabPage("spheres");
      spheresLayout = new SpheresTabPage();
      spheresLayout.Dock = DockStyle.Fill;
      spheresLayout.treeView1.DoubleClick += new EventHandler(spheresTreeViewDoubleClick);
      spheresLayout.button1.Click += new EventHandler(addSphere);
      spheresLayout.button2.Click += new EventHandler(removeSphere);
      //Panel panel = new Panel();
      //panel.AutoScroll = true;
      //panel.Size = new Size(tabControl1.Size.Width - 5, tabControl1.Height - 10);
      tab.Controls.Add(spheresLayout);
      tabControl1.TabPages.Add(tab);
      movesFileToolStripMenuItem.Enabled = true;
      passToolStripMenuItem.Enabled = true;
    }

    private void passToolStripMenuItem_Click(object sender, EventArgs e) {
      TabPage tab = new TabPage("pass " + passCount++);
      PassTabPage passLayout = new PassTabPage();
      passLayout.Dock = DockStyle.Fill;
      passLayout.DoubleClick += new EventHandler(movesTreeViewDoubleClick);
      //Panel panel = new Panel();
      //panel.AutoScroll = true;
      //panel.Size = new Size(tabControl1.Size.Width - 5, tabControl1.Height - 10);
      tab.Controls.Add(passLayout);
      tabControl1.TabPages.Add(tab);

      passLayouts.Add(tab, passLayout);

      int sphereCount = 0;
      while (sphereCount < spheresLayout.treeView1.Nodes.Count) {
        TreeNode node = new TreeNode();
        node.Text = "sphere" + (sphereCount + 1);
        node.Tag = new Move();
        passLayout.treeView1.Nodes.Add(node);
        sphereCount++;
      }

      saveToolStripMenuItem.Enabled = true;
    }

    private void spheresFileToolStripMenuItem_Click(object sender, EventArgs e) {
      // opening file code taken from https://msdn.microsoft.com/en-gb/library/cc221415%28v=vs.95%29.aspx
      OpenFileDialog openFileDialog1 = new OpenFileDialog();

      openFileDialog1.Filter = "XML Files (.xml)|*.xml|All Files (*.*)|*.*";
      openFileDialog1.FilterIndex = 1;
      openFileDialog1.Multiselect = false;

      DialogResult result = openFileDialog1.ShowDialog();

      if (result == DialogResult.OK) {
        Cursor.Current = Cursors.WaitCursor;
        FileInfo f = new FileInfo(openFileDialog1.FileName);
        Cursor.Current = Cursors.WaitCursor;

        Stream fileStream = openFileDialog1.OpenFile();
        // XML reading code adapted from http://csharp.net-tutorials.com/xml/reading-xml-with-the-xmldocument-class/
        XmlDocument xmlDoc = new XmlDocument();
        xmlDoc.Load(fileStream);
        clearForm();

        TabPage tab = new TabPage("spheres");
        //FlowLayoutPanel panel = new FlowLayoutPanel();
        //panel.AutoScroll = true;
        //panel.Size = new Size(tabControl1.Size.Width - 5, tabControl1.Height - 50);
        //TreeView view = new TreeView();
        //view.Size = new Size(panel.Size.Width - 25, panel.Size.Height - 10);
        //view.DoubleClick += new EventHandler(spheresTreeViewDoubleClick);
        //Button b1 = new Button();
        //b1.Text = "Add Sphere";
        //b1.Click += new EventHandler(addSphere);
        //Button b2 = new Button();
        //b2.Text = "Remove Sphere";
        //b2.Click += new EventHandler(removeSphere);
        spheresLayout = new SpheresTabPage();
        spheresLayout.Dock = DockStyle.Fill;
        spheresLayout.treeView1.DoubleClick += new EventHandler(spheresTreeViewDoubleClick);
        spheresLayout.button1.Click += new EventHandler(addSphere);
        spheresLayout.button2.Click += new EventHandler(removeSphere);
        //panel.Controls.Add(view);
        //panel.Controls.Add(b1);
        //panel.Controls.Add(b2);
        tab.Controls.Add(spheresLayout);
        tabControl1.TabPages.Add(tab);

        foreach (XmlNode sphere in xmlDoc.DocumentElement.ChildNodes) {
          loadSphere(sphere);
        }

        //for (int i = 0; i < spheres.Count; i++) {
        //  TreeNode node = new TreeNode();
        //  node.Text = "sphere " + (i + 1);
        //  node.Tag = spheres[i];
        //  layout.treeView1.Nodes.Add(node);
        //}

        movesFileToolStripMenuItem.Enabled = true;
        passToolStripMenuItem.Enabled = true;
        Cursor.Current = Cursors.Default;
      }
    }

    private void movesFileToolStripMenuItem_Click(object sender, EventArgs e) {
      OpenFileDialog openFileDialog1 = new OpenFileDialog();

      openFileDialog1.Filter = "XML Files (.xml)|*.xml|All Files (*.*)|*.*";
      openFileDialog1.FilterIndex = 1;
      openFileDialog1.Multiselect = false;

      DialogResult result = openFileDialog1.ShowDialog();

      if (result == DialogResult.OK) {
        Cursor.Current = Cursors.WaitCursor;
        FileInfo f = new FileInfo(openFileDialog1.FileName);
        Cursor.Current = Cursors.WaitCursor;

        Stream fileStream = openFileDialog1.OpenFile();
        // XML reading code adapted from http://csharp.net-tutorials.com/xml/reading-xml-with-the-xmldocument-class/
        XmlDocument xmlDoc = new XmlDocument();
        xmlDoc.Load(fileStream);

        foreach (XmlNode pass in xmlDoc.DocumentElement.ChildNodes) {
          TabPage tab = new TabPage("pass " + passCount++);
          //Panel panel = new Panel();
          //panel.AutoScroll = true;
          //panel.Size = new Size(tabControl1.Size.Width - 5, tabControl1.Height - 50);
          //TreeView view = new TreeView();
          //view.Size = new Size(panel.Size.Width - 25, panel.Size.Height - 10);
          //view.DoubleClick += new EventHandler(movesTreeViewDoubleClick);
          //panel.Controls.Add(view);
          //tab.Controls.Add(panel);
          PassTabPage layout = new PassTabPage();
          layout.Dock = DockStyle.Fill;
          layout.treeView1.DoubleClick += new EventHandler(movesTreeViewDoubleClick);
          passLayouts.Add(tab, layout);
          tab.Controls.Add(layout);
          tabControl1.TabPages.Add(tab);

          layout.numericUpDown1.Text = pass.Attributes["frames"].Value;
          layout.numericUpDown2.Text = pass.Attributes["threads"].Value;
          loadPass(tab, pass);

          int i = 0;
          //while (i < spheres.Count) {
          //  TreeNode node = new TreeNode();
          //  node.Text = "sphere " + (i + 1);
          //  node.Tag = passes[tab][i];
          //  layout.treeView1.Nodes.Add(node);
          //  i++;
          //}
        }
        Cursor.Current = Cursors.Default;
        saveToolStripMenuItem.Enabled = true;
      }
    }

    private void loadSphere(XmlNode sphereNode) {
      float x = float.Parse(sphereNode.Attributes["x"].Value);
      float y = float.Parse(sphereNode.Attributes["y"].Value);
      float z = float.Parse(sphereNode.Attributes["z"].Value);
      float radius = float.Parse(sphereNode.Attributes["radius"].Value);
      float r = float.Parse(sphereNode.Attributes["r"].Value);
      float g = float.Parse(sphereNode.Attributes["g"].Value);
      float b = float.Parse(sphereNode.Attributes["b"].Value);
      float reflection = -1.0f, transparency = -1.0f, eR = -1.0f, eG = -1.0f, eB = -1.0f;

      if (sphereNode.Attributes["reflection"] != null) {
        reflection = float.Parse(sphereNode.Attributes["reflection"].Value);
      }
      if (sphereNode.Attributes["transparency"] != null) {
        transparency = float.Parse(sphereNode.Attributes["transparency"].Value);
      }
      if (sphereNode.Attributes["emissionR"] != null) {
        eR = float.Parse(sphereNode.Attributes["emissionR"].Value);
      }
      if (sphereNode.Attributes["emissionG"] != null) {
        eG = float.Parse(sphereNode.Attributes["emissionG"].Value);
      }
      if (sphereNode.Attributes["emissionB"] != null) {
        eB = float.Parse(sphereNode.Attributes["emissionB"].Value);
      }
      
      System.Windows.Media.Color emissionColor = System.Windows.Media.Color.FromScRgb(1.0f, eR == -1.0f ? 0.0f : eR, eG == -1.0f ? 0.0f : eG, eB == -1.0f ? 0.0f : eB);

      TreeNode node = new TreeNode();
      node.Text = "sphere " + sphereCount++;
      node.Tag = new Sphere(x, y, z, radius, System.Windows.Media.Color.FromScRgb(1.0f, r, g, b), emissionColor, reflection == -1.0f ? 0.0f : reflection, transparency == -1.0 ? 0.0f : transparency);
      spheresLayout.treeView1.Nodes.Add(node);
    }

    private void loadPass(TabPage tab, XmlNode passNode) {
      for (int i = 0; i < spheresLayout.treeView1.Nodes.Count; i++) {
        TreeNode node = new TreeNode();
        node.Text = "sphere " + (i + 1);
        node.Tag = new Move();
        passLayouts[tab].treeView1.Nodes.Add(node);
      }

      foreach (XmlNode move in passNode.ChildNodes) {
        int target = Int32.Parse(move.Attributes["target"].Value);
        String function = move.Attributes["function"].Value;
        float amount = float.Parse(move.Attributes["amount"].Value);

        ((Move) passLayouts[tab].treeView1.Nodes[target].Tag).setTarget(target);
        ((Move) passLayouts[tab].treeView1.Nodes[target].Tag).setFunction(function);
        ((Move) passLayouts[tab].treeView1.Nodes[target].Tag).setAmount(amount);
      }
    }

    private void saveToolStripMenuItem_Click(object sender, EventArgs e) {
      SaveFileDialog saveFileDialog1 = new SaveFileDialog();
      saveFileDialog1.Filter = "XML file|*.xml";
      saveFileDialog1.Title = "Save Tileset";
      saveFileDialog1.ShowDialog();

      if (saveFileDialog1.FileName != "") {
        Cursor.Current = Cursors.WaitCursor;

        XmlWriterSettings settings = new XmlWriterSettings();
        settings.Indent = true;
        settings.IndentChars = "\t";
        XmlWriter writer = XmlWriter.Create(saveFileDialog1.FileName.IndexOf('.') == -1 ? saveFileDialog1.FileName + ".xml" : saveFileDialog1.FileName, settings);

        writer.WriteStartDocument();
        writer.WriteStartElement("scene"); // <scene>

        foreach (TreeNode node in spheresLayout.treeView1.Nodes) {
          Sphere sphere = (Sphere) node.Tag;

          writer.WriteStartElement("sphere"); // <sphere>

          writer.WriteAttributeString("x", XmlConvert.ToString(sphere.getX()));
          writer.WriteAttributeString("y", XmlConvert.ToString(sphere.getY()));
          writer.WriteAttributeString("z", XmlConvert.ToString(sphere.getZ()));
          writer.WriteAttributeString("radius", XmlConvert.ToString(sphere.getRadius()));
          writer.WriteAttributeString("r", XmlConvert.ToString(sphere.getSurface().ScR));
          writer.WriteAttributeString("g", XmlConvert.ToString(sphere.getSurface().ScG));
          writer.WriteAttributeString("b", XmlConvert.ToString(sphere.getSurface().ScB));

          if (sphere.getReflection() > 0.0f) {
            writer.WriteAttributeString("reflection", XmlConvert.ToString(sphere.getReflection()));
          }
          if (sphere.getTransparency() > 0.0f) {
            writer.WriteAttributeString("transparency", XmlConvert.ToString(sphere.getTransparency()));
          }
          if (sphere.getEmission().ScR > 0.0f) {
            writer.WriteAttributeString("emissionR", XmlConvert.ToString(sphere.getEmission().ScR));
          }
          if (sphere.getEmission().ScG > 0.0f) {
            writer.WriteAttributeString("emissionG", XmlConvert.ToString(sphere.getEmission().ScG));
          }
          if (sphere.getEmission().ScB > 0.0f) {
            writer.WriteAttributeString("emissionB", XmlConvert.ToString(sphere.getEmission().ScB));
          }

          writer.WriteEndElement(); // </sphere>
        }

        writer.WriteEndElement(); // </scene>
        writer.Close();

        saveFileDialog1.ShowDialog();

        if (saveFileDialog1.FileName != "") {
          writer = XmlWriter.Create(saveFileDialog1.FileName.IndexOf('.') == -1 ? saveFileDialog1.FileName + ".xml" : saveFileDialog1.FileName, settings);

          writer.WriteStartDocument();
          writer.WriteStartElement("passes"); // <passes>

          for (int i = 1; i < tabControl1.TabPages.Count; i++) {
            PassTabPage layout = (PassTabPage) tabControl1.TabPages[i].Controls[0];

            writer.WriteStartElement("pass"); // <pass>

            writer.WriteAttributeString("frames", layout.numericUpDown1.Text);
            writer.WriteAttributeString("threads", layout.numericUpDown2.Text);

            foreach (TreeNode node in passLayouts[tabControl1.TabPages[i]].treeView1.Nodes) {
              Move move = (Move) node.Tag;

              if (move.getFunction() != null && !"".Equals(move.getFunction())) {
                writer.WriteStartElement("move"); // <move>

                writer.WriteAttributeString("target", XmlConvert.ToString(move.getTarget()));
                writer.WriteAttributeString("function", move.getFunction());
                writer.WriteAttributeString("amount", XmlConvert.ToString(move.getAmount()));

                writer.WriteEndElement(); // </move>
              }
            }

            writer.WriteEndElement(); // </pass>
          }

          writer.WriteEndElement(); // </passes>
          writer.Close();
        }
        Cursor.Current = Cursors.Default;
      }
    }

    /*
    * Taken from https://stackoverflow.com/questions/5011381/how-to-remove-a-tabpage-from-a-context-menu
    */
    private void tabControlMouseClick(object sender, MouseEventArgs e) {
      if (e.Button == MouseButtons.Right) {
        for (int i = 0; i < tabControl1.TabCount; i++) {
          Rectangle r = tabControl1.GetTabRect(i);
          if (r.Contains(e.Location)) {
            if (tabControl1.TabPages[i].Text != "spheres") { 
              closeTabContextMenu.Tag = tabControl1.TabPages[i];
              closeTabContextMenu.Show(tabControl1, e.Location);
            }
          }
        }
      }
    }

    /*
     * https://stackoverflow.com/questions/5011381/how-to-remove-a-tabpage-from-a-context-menu
     */
    private void closeTab(object sender, EventArgs e) {
      tabControl1.TabPages.Remove(closeTabContextMenu.Tag as TabPage);
      if (tabControl1.TabPages.Count == 1) {
        saveToolStripMenuItem.Enabled = false;
        passCount = 1;
      }
    }

    /*
     * Taken from http://dotnetrix.co.uk/tabcontrol.htm#tip7
     */
    private void tabControlMouseDown(object sender, MouseEventArgs e) {
      dragStartPosition = new Point(e.X, e.Y);
    }

    /*
     * Taken from http://dotnetrix.co.uk/tabcontrol.htm#tip7
     */
    private void tabControlMouseMove(object sender, MouseEventArgs e) {
      if (e.Button != MouseButtons.Left) {
        return;
      }

      Rectangle r = new Rectangle(dragStartPosition, Size.Empty);
      r.Inflate(SystemInformation.DragSize);

      TabPage tp = hoverTab();

      if (tp != null) {
        if (!r.Contains(e.X, e.Y)) {
          tabControl1.DoDragDrop(tp, DragDropEffects.All);
        }
      }
      dragStartPosition = Point.Empty;
    }

    /*
     * Taken from http://dotnetrix.co.uk/tabcontrol.htm#tip7
     */
    private void tabControlDragOver(object sender, DragEventArgs e) {
      TabPage tab = hoverTab();

      if (tab == null) {
        e.Effect = DragDropEffects.None;
      }
      else {
        if (e.Data.GetDataPresent(typeof(TabPage))) {
          e.Effect = DragDropEffects.Move;
          TabPage dragTab = (TabPage) e.Data.GetData(typeof(TabPage));

          if (tab == dragTab) {
            return;
          }

          Rectangle tabRect = tabControl1.GetTabRect(tabControl1.TabPages.IndexOf(tab));
          tabRect.Inflate(-3, -3);
          if (tabRect.Contains(tabControl1.PointToClient(new Point(e.X, e.Y)))) {
            swapTabPages(dragTab, tab);
            tabControl1.SelectedTab = dragTab;
          }
        }
      }
    }

    /*
     * Taken from http://dotnetrix.co.uk/tabcontrol.htm#tip7
     */
    private TabPage hoverTab() {
      for (int index = 0; index <= tabControl1.TabCount - 1; index++) {
        if (tabControl1.GetTabRect(index).Contains(tabControl1.PointToClient(Cursor.Position)))
          return tabControl1.TabPages[index];
      }
      return null;
    }

    /*
     * Taken from http://dotnetrix.co.uk/tabcontrol.htm#tip7
     */
    private void swapTabPages(TabPage tp1, TabPage tp2) {
      if (tp1.Text != "spheres" && tp2.Text != "spheres") {
        int index1 = tabControl1.TabPages.IndexOf(tp1);
        int index2 = tabControl1.TabPages.IndexOf(tp2);
        tabControl1.TabPages[index1] = tp2;
        tabControl1.TabPages[index2] = tp1;
      }
    }

    private void addSphere(object sender, EventArgs args) {
      TreeView view = ((SpheresTabPage) tabControl1.SelectedTab.Controls[0]).treeView1;
      TreeNode node = new TreeNode();
      node.Text = "sphere " + sphereCount++;
      Sphere sphere = new Sphere();
      node.Tag = sphere;
      view.Nodes.Add(node);

      for (int i = 1; i < tabControl1.TabPages.Count; i++) {
        TreeView passView = ((PassTabPage) tabControl1.TabPages[i].Controls[0]).treeView1;

        Move move = new Move();
        TreeNode moveNode = new TreeNode();
        moveNode.Text = node.Text;
        moveNode.Tag = move;
        passView.Nodes.Add(moveNode);
      }
    }

    private void removeSphere(object sender, EventArgs args) {
      TreeView view = ((SpheresTabPage) tabControl1.SelectedTab.Controls[0]).treeView1;
      TreeNode node = view.SelectedNode;

      if (node != null) {
        view.Nodes.Remove(node);

        for (int i = 1; i < tabControl1.TabPages.Count; i++) {
          TreeView passView = ((PassTabPage) tabControl1.TabPages[i].Controls[0]).treeView1;
          passView.Nodes.Remove(passView.Nodes[node.Index]);
        }
      }
    }

    private void spheresTreeViewDoubleClick(object sender, EventArgs args) {
      SpherePropertiesDialog dialog = new SpherePropertiesDialog((Sphere) ((TreeView) sender).SelectedNode.Tag);

      DialogResult result = dialog.ShowDialog();
      if (result == DialogResult.OK) {
        Sphere sphere = (Sphere) ((TreeView) sender).SelectedNode.Tag;

        sphere.setX(dialog.getX());
        sphere.setY(dialog.getY());
        sphere.setZ(dialog.getZ());
        sphere.setRadius(dialog.getRadius());
        sphere.setReflection(dialog.getReflection());
        sphere.setTransparency(dialog.getTransparency());
        sphere.setSurface(dialog.getSurface());
        sphere.setEmission(dialog.getEmission());
      }
    }

    private void movesTreeViewDoubleClick(object sender, EventArgs args) {
      MovePropertiesDialog dialog = new MovePropertiesDialog((Move) ((TreeView) sender).SelectedNode.Tag, spheresLayout.treeView1.Nodes.Count);

      DialogResult result = dialog.ShowDialog();
      if (result == DialogResult.OK) {
        Move move = (Move) ((TreeView) sender).SelectedNode.Tag;

        move.setTarget(((TreeView) sender).SelectedNode.Index);
        move.setFunction(dialog.getFunction());
        move.setAmount(dialog.getAmount());
      }
    }
  }
}
