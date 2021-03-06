grammar BPLang;

tokens {
  OR	= '+' ;
  AND	= '*' ;
  NOT	= '!' ;
}


/*------------------------------------------------------------------
 * PARSER RULES
 *------------------------------------------------------------------*/

eval returns [Node value]
  : {
      Node.log("Root node");
      Node tmp = new Node(NodeType.ROOT);
      $value = tmp;
    }

    (m1=statement  { Node.log(" m1 = eval ");
                     tmp.children.add($m1.value);
                     $value = tmp;
      }
    )*
  ;

statement returns [Node value]
  : (((m1=wire { Node.log(" WIRE m1 = var");
                 $value = $m1.value;
        }
        | m2=logic { Node.log(" LOGIC m2 = function");
                     $value = $m2.value;
        }
        | m3=config { Node.log(" CONFIG m3 = function");
                     $value = $m3.value;
        }
        )
        SEMICOLON
      )
    |
      m5=comment{ Node.log(" COMMENT");
                  $value = $m5.value;
    }
    )
  ;

comment returns [Node value]
  : {Node tmp = new Node(NodeType.COMMENT);
      $value = tmp;
    }
    '#'.*
  ;

wire returns [Node value]
  : WIRE m1=nodeID { Node.log("  WIRE m1 = NODEID");
                     Node tmp = new Node(NodeType.WIRE);
                     tmp.children.add($m1.value);
                     $value = tmp;
    }

    (','
      m2=nodeID { Node.log("  WIRE m2 = NODEID");
                  tmp.children.add($m2.value);
                  $value = tmp;
      }
    )*
  ;

logic returns [Node value]    
  : m1=nodeID { Node.log(" LOGIC");
                Node tmp = new Node(NodeType.LOGIC);
                $m1.value.type = NodeType.OUTPUT_ID;
                tmp.children.add($m1.value);
                $value = tmp;
    }
    (
      ','
      m1_1=nodeID { Node.log(" MORE OUTPUTS");
                    $m1_1.value.type = NodeType.OUTPUT_ID;
                    tmp.children.add($m1_1.value);
      }

    )*
    '='
    m2=module { Node.log(" MODULE NAME: ..."+$m2.value+"...");
                tmp.msg = $m2.value;
    }
    
    (
      '#('
      m3=param { Node.log(" PARAM");
                 tmp.children.add($m3.value);
      }
      
      (
        ','
        m4=param { Node.log(" PARAM MORE");
                   tmp.children.add($m4.value);
        }
      )*
      ')'
    )?

    (
      '{'
      m5=nodeID { Node.log(" LOGIC VAR");
                  $m5.value.type = NodeType.INPUT_ID;
                  tmp.children.add($m5.value);
      }
      (
        ','
        m6=nodeID { Node.log(" LOGIC VAR MORE");
                    $m6.value.type = NodeType.INPUT_ID;
                    tmp.children.add($m6.value);
        }
      )*
      '}'
    )?
  ;


config returns [Node value]    
  : m1=module { Node.log(" MODULE NAME: ..."+$m1.value+"...");
                Node tmp = new Node(NodeType.LOGICCONFIG);
                tmp.msg = $m1.value;
                $value = tmp;
    }
    
    (
      '#('
      m2=paramConfig { Node.log(" PARAMCONFIG");
                       tmp.children.add($m2.value);
      }
      
      (
        ','
        m3=paramConfig { Node.log(" PARAMCONFIG MORE");
                   tmp.children.add($m3.value);
        }
      )*
      ')'
    )?

    (
      '{'
      m4=nodeIDconfig { Node.log(" LOGIC VAR CONFIG");
                        tmp.children.add($m4.value);
      }
      (
        ','
        m5=nodeIDconfig { Node.log(" LOGIC VAR CONFIG MORE");
                          tmp.children.add($m5.value);
        }
      )*
      '}'
    )?
  ;


nodeID returns [Node value]
  : 
    m1=ID { Node.log(" nodeID: ID");
            Node tmp = new Node(NodeType.ID);
            tmp.msg = $m1.text;
            $value = tmp;
    }
  ;

nodeIDconfig returns [Node value]
  : 
    m1=ID { Node.log(" nodeIDconfig: ID");
            Node tmp = new Node(NodeType.IDCONFIG);
            tmp.msg = $m1.text;
            $value = tmp;
    }
    '%'
    m2=NUMBER { Node.log(" percentage");
                tmp.percentage = $m2.text;
    }
  ;

param returns [Node value]
  : 
    m1=NUMBER { Node.log(" param: ID");
                Node tmp = new Node(NodeType.PARAM);
                tmp.msg = $m1.text;
                $value = tmp;
    }
  ;

paramConfig returns [Node value]
  : 
    '['
    m1=NUMBER { Node.log(" paramConfig: upperBound");
                Node tmp = new Node(NodeType.PARAMCONFIG);
                tmp.upperBound = $m1.text;
                $value = tmp;
    }
    ':'
    m2=NUMBER { Node.log(" lowerBound");
                tmp.lowerBound = $m2.text;
    }
    ']'
    '%'
    m3=NUMBER { Node.log(" percentage");
                tmp.percentage = $m3.text;
    }
    
    
  ;

module returns [String value]
  : 
    m1=ID {
      $value = $m1.text;
    }
  ;

/*------------------------------------------------------------------
 * LEXER RULES
 *------------------------------------------------------------------*/


WIRE : 'Wire:'
  ;

SEMICOLON: ';'
  ;

ID  :    ('a'..'z' | 'A'..'Z') ('a'..'z' | 'A'..'Z' | '0'..'9' | '_' | '-')*
  ;


NUMBER	: (DIGIT)+ ;

WHITESPACE : ( '\t' | ' ' | '\r' | '\n'| '\u000C' )+ 	{ $channel = HIDDEN; } ;

fragment DIGIT	: '0'..'9' ;
