import org.antlr.runtime.*;
import java.util.*;
//import antlr.*;
import java.lang.Exception;
import java.io.*;

public class BPLangProg {
  
  /* Available functions:

  genPredictor:         Library -> String
  genCpp:               Node (Flat) -> CPP
  fileToDepTree:        String or File (String) -> Node (Dep)
  getInitialNodeString: String -> Node (Flat)
  getInitialNode:       File (String) -> Node (Flat)
  buildTree:            Node (Flat) -> Node (Dep)
  */
  public static void main(String[] args) throws Exception {

    if(args.length != 1) {
      System.out.println("ERROR: QueryProg <file path>");
      System.exit(0);
    }
    
    Node tree1 = fileToDepTree(args[0], true);
    String pred = genPredictor("library");
    genCpp(getInitialNodeString(pred));
    System.out.println("\n\n"+pred+"\n\n");
    System.out.println(fileToDepTree(pred, false));

    Node tree2 = getInitialNodeString(pred);

    Node tree3 = matePredictors(tree1, tree2);
  }
  
  public static int max(int a, int b) {
    if(a > b) {
      return a;
    }
    else {
      return b;
    }
  }

  public static int min(int a, int b) {
    if(a < b) {
      return a;
    }
    else {
      return b;
    }
  }

  // Node(Flat) + Node(Flat) -> Node(Flat)
  public static Node matePredictors(Node node1, Node node2) {
    Random rand = new Random(13);
    
    int maxChanges = max(1, min(node1.children.size(), node2.children.size())/2);
    int numChanges = rand.nextInt(maxChanges)+1;
    System.out.println("node1: "+node1.children.size()+", node2: "+node2.children.size());
    System.out.println("maxChanges: "+maxChanges+", numChanges: "+numChanges);

    Node node;
    if(rand.nextInt(2) == 0) {
      node = new Node(node1);
    }
    else {
      node = new Node(node2);
    }
    
    int changed = 0;
    while(changed < numChanges) {
      int sel = rand.nextInt(node.children.size());
      String selOutput = node.children.get(sel).children.get(0).msg;
      System.out.println(selOutput);
      int contains = 0;
      for(Node n : node.children) {
      }
      changed++;
    }
    return node;
  }
  
  // Library -> BP (String)
  public static String genPredictor(String file) throws Exception {
    Node node = getInitialNode(file);
    //System.out.println(node);
    Map<String, Node> library = genConfigMap(node);
    
    // Use constrained random method of generating a predictor
    
    Random rand = new Random(128);
    int predictorSize = rand.nextInt(10)+1; // size 1-10 elements
    List<String> vars = new ArrayList<String>();
    addInputKeywords(vars);

    List<String> libArray = new ArrayList<String>(library.keySet());
    List<Integer> libCntArray = new ArrayList<Integer>();
    for(int i = 0; i < libArray.size(); i++) {
      libCntArray.add(0);
    }
    
    // Init dependency array to empty
    List<List<String>> deps = new ArrayList<List<String>>();
    for(int i = 0; i < vars.size(); i++) {
      deps.add(new ArrayList<String>());
    }
    
    String predictor = "";
    String moduleName = "";
    String moduleOutput = "";
    Node module;
    int moduleIdx = 0;
    List<String> dep;
    for(int i = 0; i < predictorSize; i++) {
      dep = new ArrayList<String>();

      // Randomly select a structure
      moduleIdx = rand.nextInt(libArray.size());
      moduleName = libArray.get(moduleIdx);
      module = library.get(moduleName);
      moduleOutput = moduleName.toLowerCase()+"_"+libCntArray.get(moduleIdx);
      libCntArray.set(moduleIdx, libCntArray.get(moduleIdx)+1);
      
      // If last iteration, select as bp output
      if(i == predictorSize - 1) {
	moduleOutput = "prediction";
      }

      predictor += moduleOutput + " = " + moduleName;

      // Randomly select parameters
      int paramFound = 0;
      int numParam = 0;
      for(Node n : module.children) {
	if(n.type == NodeType.PARAMCONFIG) {
	  if(paramFound == 0) {
	    predictor += "#(";
	    paramFound = 1;
	  }
	  else if(numParam != 0) {
	    predictor += ", ";
	  }

	  int param = rand.nextInt(Integer.parseInt(n.upperBound)-Integer.parseInt(n.lowerBound))+Integer.parseInt(n.lowerBound);
	  
	  // Round to nearest power of 2
	  param = 1 << log2(param);
	  predictor+=Integer.toString(param);
	  numParam++;
	}
      }
      if(paramFound == 1) {
	predictor += ")";
      }
      
      // Randomly select input parameters
      int inputFound = 0;
      int numInput = 0;
      for(Node n : module.children) {
	if(n.type == NodeType.IDCONFIG) {
	  if(inputFound == 0) {
	    predictor += " {";
	    inputFound = 1;
	  }
	  else if(numInput != 0) {
	    predictor += ", ";
	  }
	  
	  // Choose "sticky" input
	  String input = "";
	  if((Integer.parseInt(n.percentage) > rand.nextInt(100)) && vars.contains(n.msg)) {
	    int found = 0;
	    while(found == 0) {
	      int testIdx = rand.nextInt(vars.size());
	      if(vars.get(testIdx).equals(n.msg) || deps.get(testIdx).contains(n.msg)) {
		input = vars.get(testIdx);
		found = 1;
	      }
	    }
	  }
	  else {
	    input = vars.get(rand.nextInt(vars.size()));
	  }
	  predictor += input;
	  
	  // Build dependency chain
	  // 1. Add immediate dependencies
	  dep.add(input);
	  
	  // 2. Add next set of dependencies
	  dep.addAll(deps.get(vars.indexOf(input)));
	  numInput++;
	}
      }
      if(inputFound != 0) {
	predictor += "}";
      }
      predictor += ";";

      // Assuming only 1 output, add to output list --- FIXME
      vars.add(moduleOutput);
      deps.add(dep);
      //System.out.println("Dependencies for "+moduleName+": "+dep);
      predictor+="\n";
    }
    
    //System.out.println(predictor);

    return predictor; //fileToDepTree(predictor, false);
  }
  
  // Node (Flat) -> CPP
  public static void genCpp(Node tree) throws Exception {
    FileWriter fstream = new FileWriter("predictor.cc");
    BufferedWriter out = new BufferedWriter(fstream);
    //System.out.println(tree);
    // Headers
    out.write("#include <stdio.h>\n");
    out.write("#include <cassert>\n");
    out.write("#include <string.h>\n");
    out.write("#include <inttypes.h>\n\n");
    
    out.write("using namespace std;\n");
    out.write("#include \"cbp3_def.h\"\n");
    out.write("#include \"cbp3_framework.h\"\n");
    out.write("#include \"predictor.h\"\n\n");
    
    // Global variable definition
    out.write("cbp3_cycle_activity_t *cycle_info;\n");
    out.write("int numFetch;\n");
    out.write("int numRetire;\n");
    out.write("uint32_t fe_ptr;\n");
    out.write("uint32_t retire_ptr;\n");
    out.write("cbp3_uop_dynamic_t *fe_uop;\n");
    out.write("cbp3_uop_dynamic_t *retire_uop;\n\n");
    
    out.write("dynamic_bitset<> readValid;\n");
    out.write("dynamic_bitset<> writeValid;\n");
    out.write("dynamic_bitset<> readPC;\n");
    out.write("dynamic_bitset<> writePC;\n");
    out.write("dynamic_bitset<> writeTaken;\n");
    out.write("dynamic_bitset<> writeMispredicted;\n\n\n");
    
    for(Node n : tree.children) {
      out.write(n.msg+" ");
      if(n.children.get(0).type == NodeType.OUTPUT_ID) {
	  out.write("module_"+n.children.get(0).msg+" = "+n.msg+"();\n");
      }
      else {
	throw new Exception("ERROR: OUTPUT_ID not found");
      }
    }
    
    // Predictor Init
    int outFound = 0;
    int paramFound = 0;
    out.write("\n\nvoid PredictorInit() {}\n\n");
/*
    out.write("\n\nvoid PredictorInit() {\n");
    for(Node n : tree.children) {
      outFound = 0;
      paramFound = 0;
      out.write("  ");
      for(Node val : n.children) {
	if((outFound == 0) && (val.type == NodeType.OUTPUT_ID)) {
	  out.write("module_"+val.msg+" = new "+n.msg+"(");
	  outFound = 1;
	}
	if(val.type == NodeType.PARAM) {
	  if(paramFound != 0) {
	    out.write(", ");
	  }
	  out.write(val.msg);
	  paramFound = 1;
	}
      }
      out.write(");\n");
    }
    out.write("}\n\n");
*/  
    // Predictor Reset
    out.write("void PredictorReset() {}\n\n");
    
    // Predictor Run a Cycle
    out.write("void PredictorRunACycle() {\n");
    out.write("  // get info about what uops are processed at each pipeline stage\n");
    out.write("  cycle_info = get_cycle_info();\n");
    
    out.write("  numFetch = cycle_info->num_fetch;\n");
    out.write("  numRetire = cycle_info->num_retire;\n");
    out.write("  for(int i = 0; i < max(numFetch, numRetire); i++) {\n");
    out.write("    fe_ptr = cycle_info->fetch_q[i];\n");
    out.write("    retire_ptr = cycle_info->retire_q[i];\n");
    out.write("    fe_uop = &fetch_entry(fe_ptr)->uop;\n");
    out.write("    retire_uop = &rob_entry(rob_ptr)->uop;\n\n");
    
    out.write("    // Assign static variables\n");
    out.write("    readValid = dynamic_bitset<>(1, 0);\n");
    out.write("    writeValid = dynamic_bitset<>(1, 0);\n");
    out.write("    if((i < numFetch) && (fe_uop->type & IS_BR_CONDITIONAL)) {\n");
    out.write("      readValid[0] = true;\n");
    out.write("    }\n");
    out.write("    if((i < numRetire) && (retire_uop->type & IS_BR_CONDITIONAL)) {\n");
    out.write("      writeValid[0] = true;\n");
    out.write("    }\n\n");
    
    out.write("    readPC = dynamic_bitset<>(32, fe_uop->pc);\n");
    out.write("    writePC = dynamic_bitset<>(32, retire_uop->pc);\n");
    out.write("    writeTaken = dynamic_bitset<>(1, retire_uop->br_taken);\n");
    out.write("    writeMispredicted = dynamic_bitset<>(1, 0);\n");
    out.write("    writeMispredicted[0] = (rob_entry(rob_ptr)->last_pred == retire_uop->br_taken);\n\n");
    
    out.write("    dynamic_bitset<> prediction = dynamic_bitset<>(1,0);\n\n");
    out.write("    // For special modules, some additional processing may be necessary\n");
    out.write("    updateSpecialState();\n\n");
    out.write("    // ---------------- GENERATED LOGIC HERE -----------------\n\n\n");
    int inputFound = 0;
    for(Node n : tree.children) {
      outFound = 0;
      inputFound = 0;
      out.write("    ");
      for(Node val : n.children) {
	if((outFound == 0) && (val.type == NodeType.OUTPUT_ID)) {
	  
	  out.write("dynamic_bitset<> "+val.msg+" = module_"+val.msg+".Invocate(");
	  outFound = 1;
	}
	if(val.type == NodeType.INPUT_ID) {
	  if(inputFound != 0) {
	    out.write(", ");
	  }
	  out.write(val.msg);
	  inputFound = 1;
	}
      }
      out.write(");\n");
    }
    
    // Wrap everything up
    
    out.write("\n\n\n    // Report prediction\n");
    out.write("    if(readValid[0]) {\n");
    out.write("      assert(report_pred(fe_ptr, false, prediction[0]));\n");
    out.write("    }\n");
    out.write("  }\n");
    out.write("}\n\n\n");

    out.write("void PredictorRunEnd() {}\n\n");

    out.write("void PredictorExit() {}\n\n");


    out.close();
  }


  public static int log2(int val) {
    int x = 0;

    while((val >>= 1) > 0) {
      x++;
    }

    return x;
  }
  
  
  public static Map<String, Node> genConfigMap(Node node) {
    Map<String, Node> map = new HashMap<String, Node>();

    for(Node n : node.children) {
      map.put(n.msg, n);
    }
    return map;
  }
  
  // String or File (String) -> Node (Dep)
  public static Node fileToDepTree(String file, boolean isFile) throws Exception {
    Node node;
    if(isFile) {
      node = getInitialNode(file);
    }
    else {
      node = getInitialNodeString(file);
    }

    //System.out.println("\nInitial tree structure:");
    //System.out.println(node.toString());
	
    // Generate new tree based off this structure
    List<String> inputKeywords = new ArrayList<String>();
    addInputKeywords(inputKeywords);
    Node nodeTree = buildTree("prediction", node, inputKeywords);
    
    //System.out.println(nodeTree.toString());
    
    return nodeTree;
  }
  
  // String -> Node (Flat)
  public static Node getInitialNodeString(String str) throws Exception {
    ANTLRStringStream in = new ANTLRStringStream(str);
    BPLangLexer lexer = new BPLangLexer(in);
    CommonTokenStream tokens = new CommonTokenStream(lexer);
    BPLangParser parser = new BPLangParser(tokens);
    Node node = parser.eval();
    
    return node;
  }

  // File (String) -> Node (Flat)
  public static Node getInitialNode(String file) throws Exception {
    ANTLRStringStream in = new ANTLRFileStream(file);
    BPLangLexer lexer = new BPLangLexer(in);
    CommonTokenStream tokens = new CommonTokenStream(lexer);
    BPLangParser parser = new BPLangParser(tokens);
    Node node = parser.eval();
    
    return node;
  }

  public static void addInputKeywords(List<String> list) {
    list.add("readValid");
    list.add("readPC");
    list.add("writeValid");
    list.add("writePC");
    list.add("writeTaken");
    list.add("writeMispredicted");
  }

  public static void removeSpecialInputs(List<String> list, List<String> inputKeywords) {

    int i = 0;
    int reset = 0;
    while(i < list.size()) {
      for(String line : inputKeywords) {
	if(list.get(i).equals(line)) {
	  list.remove(i);
	  reset = 1;
	  break;
	}
      }

      if(reset == 1) {
	i = 0;
	reset = 0;
      }
      else {
	i = i + 1;
      }
    }
  }

  // Node (Flat) -> Node (Dep)
  public static Node buildTree(String outputName, Node rawNodes, List<String> filter) throws Exception {  
    boolean predictionFound = false;
    //System.out.println("Finding node corresponding to output: "+outputName);

    Node newNode = null;
    List<String> inputs = new ArrayList<String>();

    // Find node that outputs to "prediction"
    for(Node module : rawNodes.children) {
      // Each module contains 1+ OUTPUT_ID, 0+ PARAM, 0+ INPUT_ID
      // For each module, examine the output_ids
      for(Node id : module.children) {
	if(id.type == NodeType.OUTPUT_ID && id.msg.equals(outputName)) {
	  //System.out.println("Prediction found in module "+module.msg);
	  
	  // Set root node in new tree to point to this node
	  newNode = new Node(NodeType.LOGIC);
	  newNode.instanceName = outputName;
	  newNode.msg = module.msg;
	  newNode.nodePtr = module;
	  
	  predictionFound = true;
	}
	
	if(predictionFound && id.type == NodeType.INPUT_ID) {
	  inputs.add(id.msg);
	}
      }
      predictionFound = false;
    }

    if(newNode == null) {
      throw new Exception("ERROR: Output "+outputName+" not found!");
    }

    // Clean inputs
    removeSpecialInputs(inputs, filter);
    
    // for each remaining input, add the module that outputs it to newNode as children
    for(String input : inputs) {
      newNode.children.add(buildTree(input, rawNodes, filter));
    }

    return newNode;
  }
}
