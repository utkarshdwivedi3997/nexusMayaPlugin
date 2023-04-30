import maya.cmds as mc
import maya.mel as mel

# Delete menu if it already exists
if mc.menu("NexusMenu", exists=True):
        mc.deleteUI("NexusMenu")
        
nexusMenu = mc.menu("NexusMenu",label="Nexus",parent=mel.eval("$retvalue = $gMainWindow;"))
mc.menuItem(label="Create Nexus Cloth", command="createNexusObject(0)", parent=nexusMenu)
mc.menuItem(label="Create Nexus Rigidbody", command="createNexusObject(1)", parent=nexusMenu)
mc.menuItem(label="Attach Vertices to Locator", command="attachVertsToLocator()", parent=nexusMenu)
mc.menuItem(label="Attach Selected Vertices", command="attachSelectedVertices()", parent=nexusMenu)
    
# int nexusObjectType
# 0 = cloth
# 1 = rigidbody
def createNexusObject(nexusObjectType):
    sel = mc.ls(sl=True,type="transform")
    shapes = mc.listRelatives(sel, s=True)
    if (shapes is None or len(shapes) == 0):
        mc.confirmDialog(t="Create Nexus Object Error", m="You have no selected meshes. Please select at least 1 mesh to convert to a Nexus Object!", b="OK")
    if (nexusObjectType == 0):
        createNexusClothes(shapes)
    elif (nexusObjectType == 1):
        createNexusRigidbodies(shapes)

# Function to create NexusCloth objects out of the selected shapes
def createNexusClothes(shapes):
    for item in shapes:
        print("not implemented yet")
        #Create NexusClothNode for each of these shapes!

# Function to create NexusRigidbody objects out of the selected shapes
def createNexusRigidbodies(shapes):
 for item in shapes:
        print("not implemented yet")
        #Create NexusRigidbodyNode for each of these shapes!

def attachVertsToLocator():
    mc.confirmDialog(t="not implemented yet",m="not implemented yet",b="OK")

def attachSelectedVertices():
    selected = mc.ls(sl=True)    # all selected objects
    clothes = mc.ls(type="nexusClothNode")    # all created cloth nodes
    rbs = mc.ls(type="nexusRigidBodyNode")    # all created nexus rigidbodies
    all = clothes+rbs
    if (len(all) == 0):
        # no created clothes or rbs
        mc.confirmDialog(t="Nexus Constraint Error",m ="Please create at least 1 Nexus Cloth or RigidBody first!",b= "OK")
        return

    verts = mc.filterExpand(selected, expand=True,sm=31 )    # get all selected verts

    if (len(selected)==0 or verts is None or len(verts) == 0):
        # no valid selection
        mc.confirmDialog(t="Nexus Constraint Error",m ="Please select vertices on Nexus Objects to pin them.",b= "OK")
        return
        
    validCount = 0
    for vert in verts:
        # get the associated geometry with that vertex
        obj = mc.ls(vert, o=True)
        if (obj in all):
            validCount+=1
        print(obj[0])
    print(validCount)