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
mc.menuItem(label="Toggle Particle Rendering", command="toggleParticleRendering()", parent=nexusMenu)
mc.menuItem(label="Pin Selected Cloth Vertex", command="pinClothVert()", parent=nexusMenu)

#global consts
nexusSolverNode = "nexusSolver"
nexusClothNodeName = "nexusClothNode"
nexusRigidBodyNodeName = "nexusRigidBodyNode"
clothCount = 0
rigidBodyCount = 0
particleRendering = False

#setup commands
mc.currentUnit(time="80fps")
mc.playbackOptions(loop="once",maxTime=1000)
mc.createNode("nexusSolverNode", name = nexusSolverNode)
mc.connectAttr("time1.outTime", f"{nexusSolverNode}.timeStep")
mc.polySphere(name = "nexusSphere")
mc.hide("nexusSphere")
mc.setAttr("polySphere1.radius",0.25)
mc.instancer(name = "nexusInstancer")
mc.connectAttr("nexusSphere.matrix", "nexusInstancer.inputHierarchy[0]")

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
    global clothCount
    for item in shapes:
        #Create NexusClothNode for each of these shapes!
        clothCount = clothCount + 1
        mc.createNode(nexusClothNodeName, name = f"{nexusClothNodeName}{clothCount}")
        mc.connectAttr(f"{item}.worldMesh", f"{nexusClothNodeName}{clothCount}.inputMesh")
        mc.connectAttr( f"{nexusClothNodeName}{clothCount}.clothAttributes", f"{nexusSolverNode}.inCloths[{clothCount-1}]")
        mc.createNode("transform",name=f"nexusCloth{clothCount}")
        mc.createNode("mesh",name=f"NexusClothShape{clothCount}",parent=f"nexusCloth{clothCount}")
        mc.sets(f"NexusClothShape{clothCount}", add="initialShadingGroup")
        mc.connectAttr(f"{nexusSolverNode}.outCloths[{clothCount-1}]", f"NexusClothShape{clothCount}.inMesh")

# Function to create NexusRigidbody objects out of the selected shapes
def createNexusRigidbodies(shapes):
    global rigidBodyCount
    for item in shapes:
        #Create NexusRigidbodyNode for each of these shapes!
        rigidBodyCount = rigidBodyCount + 1
        mc.createNode(nexusRigidBodyNodeName, name = f"{nexusRigidBodyNodeName}{rigidBodyCount}")
        mc.connectAttr(f"{item}.worldMesh", f"{nexusRigidBodyNodeName}{rigidBodyCount}.inputMesh")
        mc.connectAttr( f"{nexusRigidBodyNodeName}{rigidBodyCount}.rbAttributes", f"{nexusSolverNode}.inRBs[{rigidBodyCount-1}]")
        mc.createNode("transform",name=f"nexusRigidBody{rigidBodyCount}")
        mc.createNode("mesh",name=f"NexusRigidBodyShape{rigidBodyCount}",parent=f"nexusRigidBody{rigidBodyCount}")
        mc.sets(f"NexusRigidBodyShape{rigidBodyCount}", add="initialShadingGroup")
        mc.connectAttr(f"{nexusSolverNode}.outRBs[{rigidBodyCount-1}]", f"NexusRigidBodyShape{rigidBodyCount}.inMesh")

def toggleParticleRendering():
    global particleRendering
    particleRendering = not particleRendering
    if particleRendering:
        mc.connectAttr(f"{nexusSolverNode}.oGeom", "nexusInstancer.inputPoints")
    else:
        mc.disconnectAttr(f"{nexusSolverNode}.oGeom", "nexusInstancer.inputPoints")

def pinClothVert():
    sel = mc.ls(selection=True)
    node, component = sel[0].split(".")
    vertex_index = int(component.split("[")[1].rstrip("]"))
    nodeIdx = node.split("nexusCloth")[1]
    mc.pinNexusVert(node=nexusSolverNode, clothId=int(nodeIdx)-1, vertId=int(vertex_index))

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