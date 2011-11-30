import org.antlr.runtime.*;
import java.util.*;
//import antlr.*;
import java.lang.Exception;

public class BPLangProg {


  public static void main(String[] args) throws Exception {

    if(args.length != 1) {
      System.out.println("ERROR: QueryProg <file path>");
      System.exit(0);
    }
    
    //Node tree1 = fileToDepTree(args[0], true);
    Node tree = genPredictor("library");
    System.out.println(tree);
  }
  
  public static Node genPredictor(String file) throws Exception {
    Node node = getInitialNode(file);
    System.out.println(node);
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

    String predictor = "";
    String moduleName = "";
    String moduleOutput = "";
    Node module;
    String moduleInputs = "";
    int moduleIdx = 0;
    for(int i = 0; i < predictorSize; i++) {
      moduleInputs = "";

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
	  
	  if((Integer.parseInt(n.percentage) > rand.nextInt(100)) && vars.contains(n.msg)) {
	    predictor += n.msg;
	  }
	  else {
	    predictor += vars.get(rand.nextInt(vars.size()));
	  }
	  numInput++;
	}
      }
      if(inputFound != 0) {
	predictor += "}";
      }

      predictor += ";";

      // Assuming only 1 output, add to output list --- FIXME
      vars.add(moduleOutput);
      
      predictor+="\n";
    }
    
    // Assign the prediction output value
    
    System.out.println(predictor);

    return fileToDepTree(predictor, false);
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
  
  public static Node fileToDepTree(String file, boolean isFile) throws Exception {
    Node node;
    if(isFile) {
      node = getInitialNode(file);
    }
    else {
      node = getInitialNodeString(file);
    }

    System.out.println("\nInitial tree structure:");
    System.out.println(node.toString());
	
    // Generate new tree based off this structure
    List<String> inputKeywords = new ArrayList<String>();
    addInputKeywords(inputKeywords);
    Node nodeTree = buildTree("prediction", node, inputKeywords);
    
    System.out.println(nodeTree.toString());
    
    return nodeTree;
  }
  
  public static Node getInitialNodeString(String str) throws Exception {
    ANTLRStringStream in = new ANTLRStringStream(str);
    BPLangLexer lexer = new BPLangLexer(in);
    CommonTokenStream tokens = new CommonTokenStream(lexer);
    BPLangParser parser = new BPLangParser(tokens);
    Node node = parser.eval();
    
    return node;
  }

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
