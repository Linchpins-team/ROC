package rbtree

import "fmt"

func (n *Node) String(tree *Tree) string {
	if n == tree.NIL {
		return ""
	}
	s := ""
	s += fmt.Sprint(n.Value())
	if n.Color == BLACK {
		s = "*" + s + "*"
	}
	if n.Left != tree.NIL {
		s = n.Left.String(tree) + " " + s
	}
	if n.Right != tree.NIL {
		s += " " + n.Right.String(tree)
	}
	return "(" + s + ")"
}
