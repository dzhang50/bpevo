import java.util.List;
import java.util.ArrayList;
import java.util.Set;
import java.util.HashSet;

public class Node {
  public static Set<Node> nodeSet = new HashSet<Node>();
  public static StringBuffer sb = new StringBuffer();
  public static void log( String s ) { 
    //System.out.println( s ) ; 
    sb.append( s );
  }
  public NodeType type;
  public double value;
  public String msg;
  public List<Node> children;

  public Node( NodeType type ) {
    this.type = type;
    this.children = new ArrayList<Node>();
  }

  public Node( NodeType type, Node... kids ) {
    this.type = type;
    this.children = new ArrayList<Node>();
    for ( Node n : kids ) 
      children.add( n );
  }

  public static Node rawNode( String s ) {
    Node result = new Node( NodeType.UNDEF );
    result.msg = s;
    return result;
  }

  public static int indentLevel = 0;
  public static String indentString = " ";
  public static String indentString() { 
    StringBuffer rsb = new StringBuffer();
    for ( int i = 0 ; i < indentLevel; i++ ) {
      rsb.append( indentString );
    }
    return rsb.toString();
  }
  @Override
  public String toString() {
    String result = indentString() + type + ((msg != null) ? (":" + msg ) : "") + "\n";
    indentLevel++;
    for ( Node n : children ) {
	if(n == null) {
	    System.out.println("NULL WTF");
	}
	else {
	    result = result + n.toString();
	}
    }
    indentLevel--;
    return result;
  }

  @Override 
  public boolean equals( Object o ) {
    if ( o == null ) return false;
    if ( !(o instanceof Node) ) return false;
    Node tmp = (Node) o;
    return ( type == tmp.type && msg.equals(tmp.msg) && children.equals( tmp.children ) );
  }

  @Override
  public int hashCode() { return 42; }
}
