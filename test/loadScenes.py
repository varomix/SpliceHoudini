import hou
import unittest

class TestCase( unittest.TestCase ) :

	def setUp( self ) :
		hou.hipFile.clear( True )

	def testLoadDeformedTeapot( self ) :		
		hou.hipFile.load("./hip/pushedTeapot.hipnc")

		generator = hou.node('/obj/fabricObject1/fabricGenerator1')
		geo = generator.geometry()

		npPoints = len(geo.points())
		self.assertEqual( npPoints, 2592 )


		deformer = hou.node('/obj/fabricObject1/fabricDeformer1')
		geo = deformer.geometry()
		pt = geo.points()[0]
		self.assertAlmostEqual( pt.position()[0], 0.597287893295 )

	def testLoadMultipleMeshes( self ) :		
		hou.hipFile.load("./hip/multipleMeshOutputPorts.hipnc")

		generator = hou.node('/obj/fabricObject1/fabricGenerator1')
		geo = generator.geometry()

		npPoints = len(geo.points())
		self.assertEqual( npPoints, 11798 )


if __name__ == "__main__":
    unittest.main()


