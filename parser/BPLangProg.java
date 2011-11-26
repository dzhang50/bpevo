import org.antlr.runtime.*;
import java.util.*;
import antlr.*;

public class BPLangProg {


    public static void main(String[] args) throws Exception {

	List<String> varList = new ArrayList<String>();
	Map<String, Node> funcList = new HashMap<String, Node>();
	Map<String, Node> queryList = new HashMap<String, Node>();

	if(args.length != 1) {
	    System.out.println("ERROR: QueryProg <file path>");
	    System.exit(0);
	}

        ANTLRStringStream in = new ANTLRFileStream(args[0]);
        BPLangLexer lexer = new BPLangLexer(in);
        CommonTokenStream tokens = new CommonTokenStream(lexer);
        BPLangParser parser = new BPLangParser(tokens);
	Node node = parser.eval();
	//System.out.println(file);

	System.out.println("\nInitial tree structure:");
	System.out.println(node.toString());

    }
}
