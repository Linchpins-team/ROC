package rbtree

import (
	"math/rand"
	"testing"
	"time"

	"github.com/stretchr/testify/assert"
)

type IntValue uint64

func (n IntValue) Value() uint64 {
	return uint64(n)
}

func init() {
	rand.Seed(time.Now().UTC().UnixNano())
}

func generateTree(t *testing.T) ([]IntValue, *Tree) {
	data := make([]IntValue, 1000)
	for i := range data {
		data[i] = IntValue(i)
	}
	shuffle(data)

	// Test Insert
	tree := New()
	for _, v := range data {
		tree.Insert(v)
		t.Logf("insert %d %s\n", uint64(v), tree)
		tree.Root.Count()
	}
	return data, tree
}

func TestSearch(t *testing.T) {
	assert := assert.New(t)
	data, tree := generateTree(t)

	shuffle(data)

	// Test Search
	for _, v := range data {
		assert.Equal(v, tree.Search(v.Value()))
	}
	t.Log(tree.String())
}

// shuffle the data list to random sort
func shuffle(data []IntValue) {
	rand.Shuffle(len(data), func(i, j int) {
		data[i], data[j] = data[j], data[i]
	})
}
