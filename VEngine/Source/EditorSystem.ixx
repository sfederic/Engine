module;
import ToolkitEditorSystem;
import NativeEditorSystem;
import GlobalDefines;
export module EditorSystem;

#ifdef EDITOR
export IEditorSystem* gEditorSystem = new ToolkitEditorSystem();
#else
export IEditorSystem* gEditorSystem = new NativeEditorSystem();
#endif
