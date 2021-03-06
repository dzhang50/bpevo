import org.antlr.runtime.*;
import java.util.*;
//import antlr.*;
import java.lang.Exception;
import java.io.*;

public class BPLangProg {
  
  /* Available functions:

  genPredictor:         Library -> String
  genCpp:               String or Node (Flat) -> CPP (File)
  genBPLang:            String -> BPLang (File)
  fileToDepTree:        String or File (String) -> Node (Dep)
  getInitialNodeString: String -> Node (Flat)
  nodeToString:         Node (Flat) -> String
  getInitialNode:       File (String) -> Node (Flat)
  buildTree:            Node (Flat) -> Node (Dep)
  */
  public static void main(String[] args) throws Exception {

    if(args.length == 0) {
      System.out.println("ERROR: QueryProg <file path>");
      System.exit(0);
    }
    
    Random rand = new Random(127);
    
    // gen <root dir> <number of predictors to generate> <max size of each predictor in lines> <rand seed>
    if(args[0].equals("init")) {
      String rootDir = args[1];
      int numPredictors = Integer.parseInt(args[2]);
      int maxLines = Integer.parseInt(args[3]);
      rand = new Random(Integer.parseInt(args[4]));
      runSysCmd("mkdir "+rootDir); //r.exec("mkdir predictors");
      
      for(int i = 0; i < numPredictors; i++) {
	String pred;
	Node node;
	do {
	  pred = genPredictor("library", maxLines, rand);
	  node = getInitialNodeString(pred);
          fixOutput(node);
          fixInputs(node, maxLines, rand);
          //System.out.println("Pred "+i+" initialized to: \n"+node);
	} while(verifyPredictor(node) == false || verifyGenPredictor(node) == false);
	
	runSysCmd("mkdir "+rootDir+"/predictor_"+i);
	//System.out.println("Raw: \n"+pred);
	genBPLang(rootDir+"/predictor_"+i+"/bplang", pred);
	pareTree(node, "prediction");
	pred = nodeToString(node);
        //System.out.println("Pared: \n"+pred);
	genCpp(rootDir+"/predictor_"+i+"/predictor.cc",pred);
	runSysCmd("cp modules/predictor.h "+rootDir+"/predictor_"+i+"/");
	
      }
    }
    // mate <path to mother> <path to father> <path to child directory> <num mutations> <rand seed>
    else if(args[0].equals("mate")) {
      Node tree1 = getInitialNode(args[1]);
      Node tree2 = getInitialNode(args[2]);
      String childPath = args[3];
      int numMutations = Integer.parseInt(args[4]);
      int maxLines = Integer.parseInt(args[5]);
      rand = new Random(Integer.parseInt(args[6]));
      
      Node child;
      do {
	//child = matePredictors(tree1, tree2, rand);

	child = treeCrossover(tree1, tree2, rand);
        //System.out.println("Mutating: \n"+child);
        for(int i = 0; i < numMutations; i++) {
	  mutatePredictor(child, "library", maxLines, rand);
	}
      } while(verifyPredictor(child) == false);
      //System.out.println("Predictor successfully mutated: \n"+child);
      
      String n = nodeToString(child);
      
      //System.out.println(n);

      //System.out.println("New predictor: \n"+n);

      genBPLang(childPath+"/bplang", n);
      pareTree(child, "prediction");
      n = nodeToString(child);
      genCpp(childPath+"/predictor.cc", n);

      //r.exec("mv predictor.cc "+childPath);
      //r.exec("mv bplang "+childPath);
      
      runSysCmd("cp modules/predictor.h "+childPath);
    }

    // view <path to bplang>
    else if(args[0].equals("view")) {
      Node tree = getInitialNode(args[1]);
      List<String> inputKeywords = new ArrayList<String>();
    
      // The more keywords added, the less depth created in general
      addInputKeywords(inputKeywords);
      Node nodeTree = buildTree("prediction", tree, inputKeywords, null);
      System.out.println(nodeTree);
    }
    // gen <path to bplang> <path to file output>
    else if(args[0].equals("gen")) {
      Node node = getInitialNode(args[1]);
      String genPath = args[2];
      pareTree(node, "prediction");
      String n = nodeToString(node);
      genCpp(genPath, n);
    }

    /*
    Node tree1 = getInitialNode(args[0]);
    String pred = genPredictor("library", rand);
    System.out.println("\n\n"+pred+"\n\n");
    //System.out.println(fileToDepTree(pred, false));

    Node tree2 = getInitialNodeString(pred);

    Node tree3;

    do {
    tree3 = matePredictors(tree1, tree2, rand);
    } while(verifyPredictor(tree3) == false);

    System.out.println(tree1+"\n+\n"+tree2+"\n=\n"+tree3);
    genCpp(tree3);
    */
  }
  
  public static void runSysCmd(String cmd) throws Exception{
    Runtime rt = Runtime.getRuntime();
    Process proc = rt.exec(cmd);
    InputStream is = proc.getInputStream();
    InputStreamReader isr = new InputStreamReader(is);
    BufferedReader br = new BufferedReader(isr);
     
    String line;
    while ((line = br.readLine()) != null) {  
      System.out.println(line);  // Prints the error lines
    }
 
    //int exitVal = proc.waitFor();   
    
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
  public static Node matePredictors(Node node1, Node node2, Random rand) throws Exception {
    
    int maxChanges = max(1, min(node1.children.size(), node2.children.size())/2);
    int numChanges = maxChanges; //rand.nextInt(maxChanges)+1;
    //System.out.println("node1: "+node1.children.size()+", node2: "+node2.children.size());
    //System.out.println("maxChanges: "+maxChanges+", numChanges: "+numChanges);

    Node node, nodeSrc;
    if(rand.nextInt(2) == 0) {
      node = new Node(node1);
      nodeSrc = node2; // not modifying, so no deep copy
    }
    else {
      node = new Node(node2);
      nodeSrc = node1;
    }
    
    List<Integer> idxQueue1 = new ArrayList<Integer>();
    List<Integer> idxQueue2 = new ArrayList<Integer>();

    int chooseRand = 1;
    int chooseIdx1 = -1;
    int chooseIdx2 = -1;
    for(int changed = 0; changed < numChanges; changed++) {
      int idx1 = -1;
      int idx2 = -1;

      // Choose a node from nodeSrc to add to node
      if(idxQueue2.size() == 0) {
	idx2 = rand.nextInt(nodeSrc.children.size());
      }
      else {
	idx2 = idxQueue2.get(0);
	idxQueue2.remove(0);
      }
      String selOutput = nodeSrc.children.get(idx2).children.get(0).msg;

      // See if the node chosen from nodeSrc exists in "node"
      for(int i = 0; i < node.children.size(); i++) {
	Node n = node.children.get(i);
	if(n.children.get(0).msg.equals(selOutput)) {
	  //System.out.println("Found "+n.children.get(0).msg+" in "+n.msg+" on line "+i);
	  idx1 = i;
	}
      }
      
      if(idx1 != -1) {
	//System.out.println("Replacing node1["+idx1+"] with node2["+idx2+"]");
	node.children.set(idx1, new Node(nodeSrc.children.get(idx2)));
      }
      else {
	//System.out.println(selOutput+" doesn't exist, adding a new node");
	if(idxQueue1.size() == 0) {
	  idx1 = rand.nextInt(node.children.size());
	} else {
	  //System.out.println("Special module chosen: "+nodeSrc.children.get(idx2));
	  //System.out.println("ChooseIdx1: "+chooseIdx1);
	  idx1 = rand.nextInt(idxQueue1.get(0)+1);
	  idxQueue1.remove(0);
	}
	node.children.add(idx1, new Node(nodeSrc.children.get(idx2)));
      }
      
      // Check result to make sure there are no loops or illegal inputs
      List<String> validWires = new ArrayList<String>();
      List<String> allWires = new ArrayList<String>();
      addInputKeywords(validWires);
      addInputKeywords(allWires);
      Node tmp = new Node(node);
      tmp.children = tmp.children.subList(0, idx1);
      
      validWires.addAll(nodeOutputs(tmp));
      allWires.addAll(nodeOutputs(node));
      //System.out.println("validWires: "+validWires);
      for(Node n : node.children.get(idx1).children) {
	if(n.type == NodeType.INPUT_ID){ 
	  //System.out.println("Considering wire "+n.msg);
	  // if a wire is not valid
	  if(!validWires.contains(n.msg)) {
	    // if defined later on in the file, or if we're already extending, or if last iteration
	    if(allWires.contains(n.msg) || chooseRand == 0 || (changed+1+idxQueue1.size() >= numChanges)) {
	      // randomly select a valid wire
	      n.msg = validWires.get(rand.nextInt(validWires.size()));
	      //System.out.println("Wire not found, randomly choosing wire "+n.msg);
	    }
	    else {
	      // Find idx
	      for(int i = 0; i < nodeSrc.children.size(); i++) {
		Node m = nodeSrc.children.get(i);
		if((m.children.get(0).type == NodeType.OUTPUT_ID) && (m.children.get(0).msg.equals(n.msg))) {
		  //chooseIdx2 = i;
		  //chooseIdx1 = idx1;
		  idxQueue2.add(i);
		  idxQueue1.add(idx1);
		  //System.out.println("Choosing special module "+m.msg+" at nodeSrc idx "+i);
		}
	      }
	    }
	  }
	}
      }
    }
    //System.out.println(nodeToString(node));
    return node;
  }
  
  public static void mutatePredictor(Node node, String file, int maxLines, Random rand) throws Exception {
    int sel = rand.nextInt(4);
    Node config = getInitialNode("library");
    
    if(sel == 0) {
      //---------- Change a wire connection -------------------
      //System.out.println("Changing a wire connection!");
      int nodeIdx = -1;
      
      // Make sure predictor has at least one input
      int numInput = 0;
      for(Node n : node.children) {
	numInput += numType(n, NodeType.INPUT_ID);
      }
      if(numInput == 0)
	return;
      
      do {
	nodeIdx = rand.nextInt(node.children.size());
      } while (numType(node.children.get(nodeIdx), NodeType.INPUT_ID) == 0);

      Node tmp = new Node(node);
      tmp.children = tmp.children.subList(0, nodeIdx);
      
      List<String> validWires = new ArrayList<String>();
      addInputKeywords(validWires);
      validWires.addAll(nodeOutputs(tmp));
      
      Node selNode = node.children.get(nodeIdx);

      int inputIdx;
      do {
	inputIdx = rand.nextInt(selNode.children.size());
      }while (selNode.children.get(inputIdx).type != NodeType.INPUT_ID);
      //System.out.print("Mutating wire "+node.children.get(nodeIdx).children.get(inputIdx).msg);
      node.children.get(nodeIdx).children.get(inputIdx).msg = validWires.get(rand.nextInt(validWires.size()));
      //System.out.println(" to "+node.children.get(nodeIdx).children.get(inputIdx).msg);
    }
    else if(sel == 1){
      //------------ Change a parameter -----------------------
      //System.out.println("Changing a parameter!");
      int nodeIdx = -1;
      
      // Make sure predictor has at least one parameter
      int numParam = 0;
      for(Node n : node.children) {
	numParam += numType(n, NodeType.PARAM);
      }
      if(numParam == 0)
	return;

      // Find a node that has a parameter
      do {
	nodeIdx = rand.nextInt(node.children.size());
      } while(numType(node.children.get(nodeIdx), NodeType.PARAM) == 0);

      String nodeType = node.children.get(nodeIdx).msg;

      //System.out.println(config);

      // Get param bounds
      int selParam = rand.nextInt(numType(node.children.get(nodeIdx), NodeType.PARAM));
      int upper = 0, lower = 0;
      int x = 0;
      for(Node n : config.children) {
	if(n.msg.equals(nodeType)) {
	  for(Node m : n.children) {
	    if(x == selParam && m.type == NodeType.PARAMCONFIG) {
	      upper = Integer.parseInt(m.upperBound);
	      lower = Integer.parseInt(m.lowerBound);
	    }
	    x++;
	  }
	}
      }
      
      // Set parameter
      x = 0;
      int tmp;
      int newVal=0;
      for(Node n : node.children.get(nodeIdx).children) {
	if(n.type == NodeType.PARAM) {
	  if(x == selParam) {
	    tmp = rand.nextInt(upper-lower+1) + lower;
	    newVal = 1 << log2(tmp);

	    //oldVal = Integer.parseInt(n.msg);
	    n.msg = Integer.toString(newVal);
	  }
	  x++;
	}
      }
      
      //System.out.println("CHANGE PARAM: "+nodeType+", upper: "+upper+", lower: "+lower+", oldVal: "+oldVal+", newVal: "+newVal);
    }
    else if(sel == 2){
      //---------------- Add a node ------------------------
      //System.out.println("Adding a node!");
      
      // Generate a 1-line predictor and add it 
      Node lib = getInitialNode(file);
      Map<String, Node> library = genConfigMap(lib);
      
      // Create past vars array
      Node tmp = new Node(node);
      int nodeIdx = tmp.children.size();
      tmp.children = tmp.children.subList(0, nodeIdx);
      
      List<String> vars = new ArrayList<String>();
      addInputKeywords(vars);
      List<String> outputs = nodeOutputs(tmp);
      vars.addAll(nodeOutputs(tmp));
      
      List<String> libArray = new ArrayList<String>(library.keySet());
      List<Integer> libCntArray = new ArrayList<Integer>();
      for(int i = 0; i < libArray.size(); i++) {
	libCntArray.add(0);
      }
      
      List<List<String>> deps = new ArrayList<List<String>>();
      
      int x = 0;
      Node newNode = getInitialNodeString(genModule(vars, library, libCntArray, deps, false, rand));
      while(outputs.contains(newNode.children.get(0).msg)){
	newNode = getInitialNodeString(genModule(vars, library, libCntArray, deps, false, rand));
	int val = libCntArray.get(x)+1;
	libCntArray.set(x, val);
	if(x <= libCntArray.size()-2)
	  x++;
	else
	  x = 0;
      }
      node.children.add(node.children.size(), newNode.children.get(0));
    }
    else {
      //---------------- Delete a node ------------------------
      //System.out.println("Deleting a node!");
      if(node.children.size() <= 2) {
	return;
      }
      node.children.remove(rand.nextInt(node.children.size()));
    }
    fixOutput(node);
    fixInputs(node, maxLines, rand);
    //System.out.println("\n\n"+nodeToString(node));
  }


  // n1 is the destination
  public static Node treeCrossover(Node nodeDest, Node nodeSrc, Random rand) throws Exception{
    Node n1, n2;
    if(rand.nextInt(2) == 0) {
      n1 = new Node(nodeDest);
      n2 = nodeSrc; // not modifying, so no deep copy
    }
    else {
      n1 = new Node(nodeSrc);
      n2 = nodeDest;
    }
    
    List<String> vars = new ArrayList<String>();
    addInputKeywords(vars);
    Node node1 = buildTree("prediction", n1, vars, null);
    Node node2 = buildTree("prediction", n2, vars, null);
    Node origNode1 = new Node(node1);
    Node origNode2 = new Node(node2);
  
    // Calculate the depth of the shallowest child
    int size1 = treeSize(node1);
    int size2 = treeSize(node2);
    
    // Get split locations
    int crossIdx1;
    crossIdx1 = rand.nextInt(size1);
    if(crossIdx1 > 3){
      crossIdx1 = 3;
    }

    int crossIdx2;
    crossIdx2 = rand.nextInt(size2); // 0 is root, so avoid 0
    if(crossIdx2 < size2 - 3){
      crossIdx2 = size2 - 3;
    }

    //System.out.println("size1: "+size1+", crossIdx1: "+crossIdx1);
    //System.out.println("size2: "+size2+", crossIdx2: "+crossIdx2);
    
    // getTreeIdx function finds the node at the layer "Idx" (new root)
    Node cross1 = getTreeIdx(node1, new ArrayList<Integer>(Arrays.asList(0)), crossIdx1);
    Node cross2 = getTreeIdx(node2, new ArrayList<Integer>(Arrays.asList(0)), crossIdx2);

    // cross1 is the node getting replaced
    //System.out.println("Cross 1:");
    //System.out.println(cross1);
    //System.out.println("Cross 2:");
    //System.out.println(cross2);

    // 
    // Swap    
    //System.out.println("Initial1:\n"+node1);
    //System.out.println("Initial2:\n"+node2);
    //System.out.println("Swap1:\n"+cross1);
    //System.out.println("Swap2:\n"+cross2);
    
    swapNode(node1, cross1, cross2);
    
    //System.out.println("Swapped:\n"+node1);
    
    // Remove branch from n1
    //System.out.println(n1);
    
    List<String> removeNodes = new ArrayList<String>();
    // Remove nodes after the Idx node of the first tree
    getNodeList(cross1, removeNodes);
    int removeIdx = -1;
    int nodeIdx = -1;
    String removeOutput = cross1.instanceName;
    for(int i = 0; i < n1.children.size(); i++) {
      //System.out.println(n1.children.get(i).instanceName);
      if(removeNodes.contains(n1.children.get(i).children.get(0).msg)) {
	n1.children.remove(i);
	
	// Find root in sub-tree
	if(i > removeIdx) {
	  //System.out.println("Found new root: "+i);
	  removeIdx = i;
	}
      }
    }
    //System.out.println(removeNodes);
    

    // Find a free output name to use
    int nameCntr;
    String addOutput;
    int j=0;
    int first = 1;
    Node toAdd = new Node(n2);
    
    //System.out.println(toAdd);
    //System.out.println(n2);
    
    pareTree(toAdd, cross2.instanceName);
    
    //System.out.println("Instance: "+cross2.instanceName);
    //System.out.println("Node: "+toAdd.msg.toLowerCase());
    //System.out.println("MSG 0: "+toAdd.children.get(0).msg);
    
    for (Node n : toAdd.children) {
      nameCntr = 0;
      
      //System.out.println("Child: "+n.msg.toLowerCase()+"\n");
      //System.out.println(n1);
      
      do {
        
        //System.out.println(n.msg.toLowerCase()+"_"+nameCntr);
        
        addOutput = toAdd.children.get(j).msg.toLowerCase()+"_"+nameCntr;
	nameCntr++;
      } while(nameInUse(n1, addOutput));
    
      // Replace old output name with new output name from n2
      //System.out.println(cross2.instanceName);
      //System.out.println(toAdd.children.get(j).instanceName);
      
      if(j == toAdd.children.size()-1){ //previously cross2.instanceName.equals(toAdd.children.get(j))
        
        //System.out.println("Removing "+removeOutput+", added at "+addOutput);

	renameInput(n1, removeOutput, addOutput);
	first = 0;
      }
      toAdd.children.get(j).children.get(0).msg = addOutput;
      n1.children.add(removeIdx+j, new Node(toAdd.children.get(j)));
      j++;
      //System.out.println(n1);
    }
    
    //System.out.println(n1);
   
    // Cleanup
    
    return n1;
  }
  
  public static boolean nameInUse(Node tree, String name) {
    for(Node n : tree.children) {
      for(Node m : n.children) {
	if((m.type == NodeType.OUTPUT_ID) && m.msg.equals(name)) {
	  return true;
	}
      }
    }
    return false;
  }
  
  public static void renameInput(Node tree, String oldName, String newName) {
    for(int i = 0; i < tree.children.size(); i++) {
      Node n = tree.children.get(i);
      for(int j = 0; j < n.children.size(); j++) {
	Node m = n.children.get(j);
	if(m.type == NodeType.INPUT_ID && m.msg.equals(oldName)) {
	  m.msg = newName;
	}
      }
    }
  }


  // Node (dep) -> List<String>
  public static void getNodeList(Node depTree, List<String> newList) {
    newList.add(depTree.instanceName);
    if(depTree.children.size() == 0) {
      return;
    }

    for(Node n : depTree.children) {
      getNodeList(n, newList);
    }
    return;
  }

  public static void swapNode(Node tree, Node nodeLoc, Node nodeTarget) {
    if(tree.children.size() == 0) {
      return;
    }

    //for(Node n : tree.children) {
    for(int i = 0; i < tree.children.size(); i++) {
      if(tree.children.get(i).instanceName.equals(nodeLoc.instanceName)) {
	//System.out.println(tree.children.get(i).instanceName);
        //System.out.println(nodeLoc.instanceName);
        tree.children.set(i, nodeTarget);
	return;
      }
    }
    
    for(Node n : tree.children) {
      swapNode(n, nodeLoc, nodeTarget);
    }
  }


  public static Node getTreeIdx(Node node, List<Integer> curIdx, int targetIdx) {
    //System.out.println(curIdx+"/"+targetIdx);
    if(curIdx.get(0) == targetIdx) {
      return node;
    }
    
    for(Node n : node.children) {
      curIdx.set(0,curIdx.get(0)+1); // curIdx++
      Node found = getTreeIdx(n, curIdx, targetIdx);
      if(found != null) {
	return found;
      }
    }

    return null;
  }


  public static void fixInputs(Node node, int maxLines, Random rand){
    int found, q, r;
    int sw1 = 0; 
    int elements = 0;
    int delete = 0;
    String loop = "";

    List<String> outputs = new ArrayList<String>();
    List<String> keywords = new ArrayList<String>();
    addInputKeywords(keywords);

    if(node.children.size() < 4){
      System.out.println("Warning: predictor length= "+node.children.size());
    }

    // Make sure predictor is short enough
    while(node.children.size()>maxLines){
      //System.out.println("REMOVING"+node.children.get(12).children.get(0).msg);
      //System.out.println("Before :\n"+node);
      node.children.remove(maxLines);
      node.children.get(maxLines-1).children.get(0).msg = "prediction";
    } 
 
    //for (Node id: node.children.get(node.children.size()-1).children){
    //  if (id.type == NodeType.OUTPUT_ID){
    //      id.msg = "prediction";  
    //  }
    //}
    
    for(Node n : node.children) {
      // No more modules
      if (n == null){
        return;
      }
      for(Node id : n.children){
        found = 0;
        if (id.type == NodeType.INPUT_ID){
          for(String k : keywords){
            if (id.msg.equals(k)){
              found = 1;
            }
          }

          for(int i=0; i<outputs.size() && found==0; i++){
            if (id.msg.equals(outputs.get(i))){
              found = 1;
              outputs.remove(i); // Remove output so it is not reused
              i--;
              loop = id.msg;
              while (checkLoop(node, sw1, loop, 0) == 1){
                // Reassigns current input due to contension 
                //System.out.println("Loop found at "+sw1+"!");
                //System.out.println(node);
                r = rand.nextInt(outputs.size() + keywords.size());
		q = rand.nextInt(outputs.size() + keywords.size());
		    // 1/2 chance of using a module and not a keyword
                if (1 > (r % 2)){
		  if (outputs.size() != 0){
                    id.msg = outputs.get(q%(outputs.size()));
		  }else{
		    id.msg = keywords.get(q%(keywords.size()));
 		  }
                }else{
                  id.msg = keywords.get(q%(keywords.size()));
                }
              }
            }
          }

          if (found == 0){
            //System.out.println("Invalid Input! "+id.msg);
            r = rand.nextInt(outputs.size() + keywords.size());
            q = rand.nextInt(outputs.size() + keywords.size());
            if (1 > (r % 2)){
              if (outputs.size() != 0){
                id.msg = outputs.get(q%(outputs.size()));
              }else{
                id.msg = keywords.get(q%(keywords.size()));
              }
            }else{
              id.msg = keywords.get(q%(keywords.size()));
            }
          }

          /*while (checkLoop(node, sw1, loop, 0) == 1){
            // Reassigns current input due to contension 
            System.out.println("Loop found at "+sw1+"!");
            System.out.println(node);
            r = rand.nextInt(outputs.size()+keywords.size()-1);
            if (r<keywords.size()){
              id.msg = keywords.get(r);
            } else{
              id.msg = outputs.get(r-keywords.size());
            }
          }*/

        }
      }
      // Add to list of outputs
      if(n.children.get(0).type != NodeType.OUTPUT_ID){
        System.out.println("ERROR: output id not first child!\n");
      }
      outputs.add(n.children.get(0).msg);
      
      sw1++;
    }
    //System.out.println(node.children);

  }
  
  // Checks for loops in the flat tree node from position sw1, with given outputs seen so far
  // returns 1 if there is a loop, and the current input should be reassigned
  // checkLoop also checks if input is valid. if not, returns 1 to be changed
  public static int checkLoop(Node node, int sw1, String loop, int second){
    String output = "";
    int sw = 0;
    int found = 0;
    int out = 0;
    
    // Check the Output name of current node
    if (node.children.get(sw1).children.get(0).type != NodeType.OUTPUT_ID){
      System.out.println("ERROR: output id not first child!\n");}
    output = node.children.get(sw1).children.get(0).msg;

	// Check if loop exists for this node
    if (output.equals(loop) && second == 1){
      return 1;
    }
    	// Check whether another loop has been reached
	// This will be fixed in a future call to this function
    if (output.equals("looped")) {
      return 0;
    }

    second = 1;

    if (output.equals("")){
      System.out.println("Ouput not found!!! Loop check failed!");
    }

    Node check = new Node(node);
    // Check for next link that takes node as input
    for (Node n: check.children){
      for (Node id : n.children){
        // 
        if (id.type == NodeType.INPUT_ID && id.msg.equals(output)){

          // Change output of every intermediate module to "looped"
          if (n.children.get(0).type == NodeType.OUTPUT_ID){
            n.children.get(0).msg = "looped";
          }else{
            System.out.println("ERROR: OUTPUT_ID IS NOT THE FIRST ELEMENT OF NODE!");
          }

          //System.out.println("Found another link node: "+id.msg);
          // Reursively check for original missing input node
          //System.out.println("Checking:"+sw);
          
	  if (checkLoop(check, sw, loop, second) == 1){
            return 1;
          }
        }
      }
      sw++;
    }
    
    return 0;    
  }

  public static void fixOutput(Node node) {
    int j,i;
    int count;

    //System.out.println("Fixing: \n"+node);
    List<String> outputs = new ArrayList<String>();
    for (i=0; i<node.children.size()-1; i++){ // Every node except the root
      //if (node.children.get(i).children.get(0).msg.equals("prediction")){
      count = 0;
      for (j=0; j<i; j++){ // Counts previous similar modules
         
        if(node.children.get(j).msg.equals(node.children.get(i).msg)){
          //System.out.println("Counted: "+node.children.get(j).msg);
          count++;}
      }
      node.children.get(i).children.get(0).msg = node.children.get(i).msg.toLowerCase()+"_"+count;
    }
    Node lastOutput = node.children.get(node.children.size()-1).children.get(0);
    lastOutput.msg = "prediction";
  }

  // Node (Flat) -> boolean
  public static boolean verifyPredictor(Node node) throws Exception {
    /* REQUIREMENTS:

    1. Maximum depth of 12
    2. Maximum 150 nodes
    3. Maximum 40 actual nodes in use
    */

    List<String> inputKeywords = new ArrayList<String>();
    addInputKeywords(inputKeywords);
    Node nodeTree = buildTree("prediction", node, inputKeywords, null);
    //System.out.println("\nVerifying Predictor!\nDepth= "+treeDepth(nodeTree)+"\nSize=  "+treeSize(nodeTree));
    //System.out.println("Children= "+node.children.size());
    //System.out.println(node);
    if(treeDepth(nodeTree) > 12) {
      return false;
    }
    
    if(treeSize(nodeTree) > 40) {
      return false;
    }
    
    if(node.children.size() > 150) {
      return false;
    }

    return true;
  }
  
  public static boolean verifyGenPredictor(Node node) throws Exception {
    /* REQUIREMENTS:

    1. Minimum depth of 2
    2. Minimum 4 nodes in use
    */

    List<String> inputKeywords = new ArrayList<String>();
    addInputKeywords(inputKeywords);
    Node nodeTree = buildTree("prediction", node, inputKeywords, null);
    if(treeDepth(nodeTree) < 2) {
      return false;
    }
    
    if(treeSize(nodeTree) < 4) {
      return false;
    }
    

    return true;
  }

  public static List<String> nodeOutputs(Node node) {
    List<String> outputs = new ArrayList<String>();

    for(Node n : node.children) {
      for(Node tmp : n.children) {
	if(tmp.type == NodeType.OUTPUT_ID) {
	  outputs.add(tmp.msg);
	}
      }
    }
    //System.out.println("Outputs: "+outputs);
    return outputs;
  }
      
  public static int numType(Node node, NodeType t) {
    int num = 0;
    for(Node n : node.children) {
      if(n.type == t) {
	num++;
      }
    }

    return num;
  }

  public static String genModule(List<String> vars, Map<String, Node> library, List<Integer> libCntArray, List<List<String>> deps, boolean lastIter, Random rand) throws Exception {
    String predictor = "";
    List<String> dep = new ArrayList<String>();
    List<String> libArray = new ArrayList<String>(library.keySet());

    // Randomly select a structure
    int moduleIdx = rand.nextInt(libArray.size());
    String moduleName = libArray.get(moduleIdx);
    Node module = library.get(moduleName);
    String moduleOutput = moduleName.toLowerCase()+"_"+libCntArray.get(moduleIdx);
    libCntArray.set(moduleIdx, libCntArray.get(moduleIdx)+1);
      
    // If last iteration, select as bp output
    if(lastIter) {
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
	    boolean depCheck;
	    if(deps.size() > testIdx)
	      depCheck = deps.get(testIdx).contains(n.msg);
	    else
	      depCheck = false;
	    if(vars.get(testIdx).equals(n.msg) || depCheck) {
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
	if(deps.size() > vars.indexOf(input))
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
    
    return predictor;
  }


  // Library -> BP (String)
  public static String genPredictor(String file, int maxLines, Random rand) throws Exception {
    Node node = getInitialNode(file);
    //System.out.println(node);
    Map<String, Node> library = genConfigMap(node);
    
    // Use constrained random method of generating a predictor
    
    int predictorSize = rand.nextInt(maxLines)+1; 
    //System.out.println("Size chosen: "+predictorSize);
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
    for(int i = 0; i < predictorSize; i++) {
      predictor += genModule(vars, library, libCntArray, deps, (i == predictorSize - 1), rand);
    }
    
    //System.out.println(predictor);

    return predictor; //fileToDepTree(predictor, false);
  }
  
  // String -> BPLang
  public static void genBPLang(String file, String str) throws Exception {
    FileWriter fstream = new FileWriter(file);
    BufferedWriter out = new BufferedWriter(fstream);
    
    // Write bp language as comment in program
    out.write(str);
    out.close();
  }

  
  // String -> CPP
  public static void genCpp(String file, String str) throws Exception {
    //FileWriter fstream = new FileWriter("predictor.cc");
    //BufferedWriter out = new BufferedWriter(fstream);
    
    // Write bp language as comment in program
    //out.write("/*\n"+str+"*/\n\n");
    //out.close();

    genCpp(file, getInitialNodeString(str), "/*\n"+str+"*/\n\n");
  }

  // Node (Flat) -> String
  public static String nodeToString(Node tree) {
    String str = "";
    for(Node n : tree.children) {
      int outputFound = 0;
      int paramFound = 0;
      int inputFound = 0;
      for(Node desc : n.children) {
	if(desc.type == NodeType.OUTPUT_ID) {
	  if(outputFound != 0) {
	    str += ", ";
	  }
	  str += desc.msg;
	  outputFound = 1;
	}
	else if(desc.type == NodeType.PARAM) {
	  if(paramFound != 0) {
	    str += ", ";
	  }
	  else {
	    str += " = "+n.msg+"#(";
	  }
	  str += desc.msg;
	  paramFound = 1;
	}
	else if(desc.type == NodeType.INPUT_ID) {
	  if(inputFound != 0) {
	    str += ", ";
	  }
	  else {
	    if(paramFound == 0) {
	      str += " = "+n.msg+" {";
	    }
	    else {
	      str += ") {";
	    }
	  }
	  str += desc.msg;
	  inputFound = 1;
	}
      }
      if(inputFound == 0) {
	str += ");\n";
      }
      else {
	str += "};\n";
      }
    }

    return str;
  }
  
  // Node (Flat) -> CPP
  public static void genCpp(String file, Node tree, String header) throws Exception {
    FileWriter fstream = new FileWriter(file);
    BufferedWriter out = new BufferedWriter(fstream);
    //System.out.println(tree);
    // Headers
    out.write("#include <stdio.h>\n");
    out.write("#include <cassert>\n");
    out.write("#include <string.h>\n");
    out.write("#include <inttypes.h>\n\n");
    
    out.write("#include <cbp3_def.h>\n");
    out.write("#include <cbp3_framework.h>\n\n");
    //out.write("#include \"predictor.h\"\n\n");

    out.write("#ifndef TABLES\n");
    out.write("#include <table.h>\n");
    out.write("#endif\n\n");    
    out.write("#ifndef LOGIC\n");
    out.write("#include <logic.h>\n");
    out.write("#endif\n\n");
    out.write("#ifndef GHR\n");
    out.write("#include <ghr.h>\n");
    out.write("#endif\n\n");

    out.write("using namespace std;\n\n");
    
    // Global variable definition
    out.write("int numFetch;\n");
    out.write("int numRetire;\n");
    out.write("uint32_t fe_ptr;\n");
    out.write("uint32_t rob_ptr;\n");
    out.write("const cbp3_uop_dynamic_t *fe_uop;\n");
    out.write("const cbp3_uop_dynamic_t *retire_uop;\n\n");
    
    out.write("dynamic_bitset<> readValid;\n");
    out.write("dynamic_bitset<> writeValid;\n");
    out.write("dynamic_bitset<> readPC;\n");
    out.write("dynamic_bitset<> writePC;\n");
    out.write("dynamic_bitset<> writeTaken;\n");
    out.write("dynamic_bitset<> writeMispredicted;\n\n\n");
    /*
    for(Node n : tree.children) {
    out.write(n.msg+" ");
    if(n.children.get(0).type == NodeType.OUTPUT_ID) {
    out.write("module_"+n.children.get(0).msg+" = "+n.msg+"();\n");
    }
    else {
    throw new Exception("ERROR: OUTPUT_ID not found");
    }
    }
    */
    // Predictor Init
    int outFound = 0;
    int paramFound = 0;

    //out.write("\n\nvoid PredictorInit() {\n");
    for(Node n : tree.children) {
      outFound = 0;
      paramFound = 0;
      //out.write("  ");
      out.write(n.msg+" ");
      for(Node val : n.children) {
	if((outFound == 0) && (val.type == NodeType.OUTPUT_ID)) {
	  out.write("module_"+val.msg+" = "+n.msg+"(");
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
    out.write("\n\n");

    out.write("void PredictorInit() {}\n\n");
    // Predictor Reset
    out.write("void PredictorReset() {}\n\n");
    
    // Predictor Run a Cycle
    out.write("void PredictorRunACycle() {\n");
    out.write("  // get info about what uops are processed at each pipeline stage\n");
    out.write("  const cbp3_cycle_activity_t *cycle_info = get_cycle_info();\n");
    
    out.write("  numFetch = cycle_info->num_fetch;\n");
    out.write("  numRetire = cycle_info->num_retire;\n");
    out.write("  for(int i = 0; i < max(numFetch, numRetire); i++) {\n");
    out.write("    fe_ptr = cycle_info->fetch_q[i];\n");
    out.write("    rob_ptr = cycle_info->retire_q[i];\n");
    out.write("    fe_uop = &fetch_entry(fe_ptr)->uop;\n");
    out.write("    retire_uop = &rob_entry(rob_ptr)->uop;\n\n");
    
    out.write("    // Assign static variables\n");
    out.write("    readValid = dynamic_bitset<>(1, 0ul);\n");
    out.write("    writeValid = dynamic_bitset<>(1, 0ul);\n");
    out.write("    if((i < numFetch) && (fe_uop->type & IS_BR_CONDITIONAL)) {\n");
    out.write("      readValid[0] = true;\n");
    out.write("    }\n");
    out.write("    if((i < numRetire) && (retire_uop->type & IS_BR_CONDITIONAL)) {\n");
    out.write("      writeValid[0] = true;\n");
    out.write("    }\n\n");
    
    out.write("    readPC = dynamic_bitset<>(32, fe_uop->pc);\n");
    out.write("    writePC = dynamic_bitset<>(32, retire_uop->pc);\n");
    out.write("    writeTaken = dynamic_bitset<>(1, retire_uop->br_taken);\n");
    out.write("    writeMispredicted = dynamic_bitset<>(1, 0ul);\n");
    out.write("    writeMispredicted[0] = (rob_entry(rob_ptr)->last_pred == retire_uop->br_taken);\n\n");
    
    //out.write("    dynamic_bitset<> prediction = dynamic_bitset<>(1,0);\n\n");
    out.write("    // For special modules, some additional processing may be necessary\n");
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
    out.write("    }\n\n");
    out.write("    updateSpecialState();\n\n");
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
    Node nodeTree = buildTree("prediction", node, inputKeywords, null);
    
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

  public static int treeDepth(Node node) {
    if(node.children.size() == 0) {
      return 1;
    }

    int max = 0;
    for(Node n : node.children) {
      int depth = treeDepth(n);
      if(depth > max) {
	max = depth;
      }
    }

    return max+1;
  }

  public static int treeSize(Node node) {
    int S;
    if(node.children.size() == 0) {
      //System.out.println("leaf: "+node.instanceName);
      return 1;
    }

    int size = 0;
    //int i = node.children.size();
    int sizes[] = new int[node.children.size()];
    int i = 0;

    for(Node n : node.children) { 
      sizes[i] = treeSize(n); // Get size of trees rooted at the children
      i++;
    }

    for(i=0; i<node.children.size(); i++){
      if(size>sizes[i] || size == 0){
        size = sizes[i]+1;
      }
    }
    
    return size;
  }
  
  public static void pareTree(Node node, String outputRoot) throws Exception {
    List<String> inputKeywords = new ArrayList<String>();
    addInputKeywords(inputKeywords);
    Node nodeTree = buildTree(outputRoot, node, inputKeywords, null);
    
    int x = 0;
    while(x < node.children.size()) {
      if(!buildTreeContainsOut(nodeTree, node.children.get(x).children.get(0).msg)) {
	node.children.remove(x);

      }else
	x++;
    }
  }

  public static boolean buildTreeContainsOut(Node node, String outputName) {
    if(node.instanceName.equals(outputName)) {
      return true;
    }

    for(Node module : node.children) {
      if(buildTreeContainsOut(module, outputName))
	return true;
    }
    return false;
  }

  // Node (Flat) -> Node (Dep)
  public static Node buildTree(String outputName, Node rawNodes, List<String> filter, Node parent) throws Exception {  
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
	  newNode.parentNodePtr = parent;
	  
	  predictionFound = true;
	}
	
	if(predictionFound && id.type == NodeType.INPUT_ID) {
	  inputs.add(id.msg);
	}
      }
      predictionFound = false;
    }

    if(newNode == null) {
      System.out.println("ERROR!! \nNodes: \n"+rawNodes);
      throw new Exception("ERROR: Output "+outputName+" not found!");
    }

    // Clean inputs
    removeSpecialInputs(inputs, filter);
    
    // for each remaining input, add the module that outputs it to newNode as children
    for(String input : inputs) {
      newNode.children.add(buildTree(input, rawNodes, filter, newNode));
    }

    return newNode;
  }
}
