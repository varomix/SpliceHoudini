import hou
import unittest

class TestCase( unittest.TestCase ) :

	def setUp( self ) :
		
		hou.hipFile.clear( True )

	def testLoadDeformedTeapot( self ) :		
		hou.hipFile.load("./hip/pushedTeapot.hipnc")

		generator = hou.node('/obj/geo1/fabricDFG1')
		geo = generator.geometry()

		npPoints = len(geo.points())
		self.assertEqual( npPoints, 128 )


		deformer = hou.node('/obj/geo1/fabricDFGDeformer2')
		geo = deformer.geometry()
		pt = geo.points()[0]
		self.assertAlmostEqual( pt.position()[0], 0.69566559 )


if __name__ == "__main__":
    unittest.main()


