package rbtree

import (
	"math/rand"
	"testing"
	"time"

	"github.com/stretchr/testify/assert"
)

type node uint64

func (n node) Value() uint64 {
	return uint64(n)
}

func init() {
	rand.Seed(time.Now().UTC().UnixNano())
}

func generateTree() ([]node, *RBTree) {
	data := make([]node, 20)
	for i := range data {
		data[i] = node(i)
	}
	shuffle(data)

	// Test Insert
	tree := New()
	for _, v := range data {
		tree.Insert(v)
	}
	return data, tree
}

func TestSearch(t *testing.T) {
	assert := assert.New(t)
	data, tree := generateTree()

	shuffle(data)

	// Test Search
	for _, v := range data {
		assert.Equal(v, tree.Search(v.Value()))
	}
	t.Log(tree.String())
}

// shuffle the data list to random sort
func shuffle(data []node) {
	rand.Shuffle(len(data), func(i, j int) {
		data[i], data[j] = data[j], data[i]
	})
}

func TestDelete(t *testing.T) {
	assert := assert.New(t)
	data, tree := generateTree()
	t.Log(data, tree)

	shuffle(data)
	for _, v := range data {
		assert.Equal(v, tree.Search(v.Value()), "%d not found in %s\n", v.Value(), tree)
		tree.Delete(v.Value())
		t.Logf("Deleted %d, %s\n", v, tree)
		assert.Nil(tree.Search(v.Value()), "%d was found in %s\n", v.Value(), tree)
	}
}

