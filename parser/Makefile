all:
	javac -cp . NodeType.java
	javac -cp . Node.java
	java -cp antlr-3.4-complete.jar org.antlr.Tool BPLang.g
	javac -cp .:antlr-3.4-complete.jar BPLangProg.java
	java -cp .:antlr-3.4-complete.jar BPLangProg gen 10 20 1337
	mkdir -p predictors/mate
	java -cp .:antlr-3.4-complete.jar BPLangProg mate predictors/predictor_3/bplang predictors/predictor_8/bplang predictors/mate 1 42 

clean:
	-rm -rf *~
	-rm -rf *.class
	-rm -rf \#*
	-rm *Parser.java
	-rm *Lexer.java
	-rm *.tokens
