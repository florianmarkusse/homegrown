package cmake

import (
	"cmd/common"
	"cmd/common/argument"
	"fmt"
	"strings"
)

const EXECUTABLE = "cmake"

type Project int64

type ProjectStructure struct {
	CCompiler           string
	Linker              string
	Folder              string
	CodeFolder          string
	DefaultFreeStanding bool
}

type CommonConfig struct {
	CCompiler string
	Linker    string
}

var ELF = CommonConfig{
	CCompiler: "clang-19",
	Linker:    "ld.lld-19",
}

var EFI = CommonConfig{
	CCompiler: "clang-19",
	Linker:    "lld-link-19",
}

// If you add a project add it here
const KERNEL = "kernel"
const INTEROPERATION = "interoperation"
const UEFI_IMAGE_CREATOR = "uefi-image-creator"
const UEFI = "uefi"
const IMAGE_BUILDER = "image-builder"
const SHARED = "shared"
const POSIX = "posix"
const PLATFORM_ABSTRACTION = "platform-abstraction"
const X86 = "x86"
const STATUS = "status"

// and here
var kernelFolder = common.REPO_PROJECTS + KERNEL + "/"
var interoperationFolder = common.REPO_PROJECTS + INTEROPERATION + "/"
var uefiImageCreatorFolder = common.REPO_PROJECTS + UEFI_IMAGE_CREATOR + "/"
var uefiFolder = common.REPO_PROJECTS + UEFI + "/"
var imageBuilderFolder = common.REPO_PROJECTS + IMAGE_BUILDER + "/"
var sharedFolder = common.REPO_PROJECTS + SHARED + "/"
var posixFolder = common.REPO_PROJECTS + POSIX + "/"
var platformAbstractionFolder = common.REPO_PROJECTS + PLATFORM_ABSTRACTION + "/"
var x86Folder = common.REPO_PROJECTS + X86 + "/"
var statusFolder = common.REPO_PROJECTS + STATUS + "/"

// and here
var PROJECT_STRUCTURES = map[string]*ProjectStructure{
	KERNEL: &ProjectStructure{
		CCompiler:           ELF.CCompiler,
		Linker:              ELF.Linker,
		Folder:              kernelFolder,
		CodeFolder:          kernelFolder + "code",
		DefaultFreeStanding: true,
	},
	INTEROPERATION: &ProjectStructure{
		CCompiler: ELF.CCompiler,
		// No linker because it is an object / interface library
		Folder:              interoperationFolder,
		CodeFolder:          interoperationFolder + "code",
		DefaultFreeStanding: true,
	},
	UEFI_IMAGE_CREATOR: &ProjectStructure{
		CCompiler:           ELF.CCompiler,
		Linker:              ELF.Linker,
		Folder:              uefiImageCreatorFolder,
		CodeFolder:          uefiImageCreatorFolder + "code",
		DefaultFreeStanding: false,
	},
	UEFI: &ProjectStructure{
		CCompiler:           EFI.CCompiler,
		Linker:              EFI.Linker,
		Folder:              uefiFolder,
		CodeFolder:          uefiFolder + "code",
		DefaultFreeStanding: true,
	},
	IMAGE_BUILDER: &ProjectStructure{
		CCompiler:           ELF.CCompiler,
		Linker:              ELF.Linker,
		Folder:              imageBuilderFolder,
		CodeFolder:          imageBuilderFolder + "code",
		DefaultFreeStanding: false,
	},
	SHARED: &ProjectStructure{
		CCompiler: ELF.CCompiler,
		// No linker because it is an object / interface library
		Folder:              sharedFolder,
		CodeFolder:          sharedFolder + "code",
		DefaultFreeStanding: true,
	},
	POSIX: &ProjectStructure{
		CCompiler: ELF.CCompiler,
		// No linker because it is an object / interface library
		Folder:              posixFolder,
		CodeFolder:          posixFolder + "code",
		DefaultFreeStanding: false,
	},
	PLATFORM_ABSTRACTION: &ProjectStructure{
		CCompiler: ELF.CCompiler,
		// No linker because it is an object / interface library
		Folder:     platformAbstractionFolder,
		CodeFolder: platformAbstractionFolder + "code",
		// No default freestanding set because the whole purpose is to provide an integration layer for different platforms
	},
	X86: &ProjectStructure{
		CCompiler: ELF.CCompiler,
		// No linker because it is an object / interface library
		Folder:              x86Folder,
		CodeFolder:          x86Folder + "code",
		DefaultFreeStanding: true,
	},
	STATUS: &ProjectStructure{
		CCompiler: EFI.CCompiler,
		// No linker because it is an object / interface library
		Folder:              statusFolder,
		CodeFolder:          statusFolder + "code",
		DefaultFreeStanding: true,
	},
}

func getConfiguredProjects() []string {
	var result = make([]string, 0)

	for name, _ := range PROJECT_STRUCTURES {
		result = append(result, name)
	}

	return result
}

var ConfiguredProjects = getConfiguredProjects()

func BuildDirectoryRoot(project *ProjectStructure, testBuild bool) string {
	buildDirectory := strings.Builder{}
	buildDirectory.WriteString(fmt.Sprintf("%s/", project.CodeFolder))
	buildDirectory.WriteString("build/")
	if testBuild {
		buildDirectory.WriteString("test/")
	} else {
		buildDirectory.WriteString("prod/")
	}
	buildDirectory.WriteString(fmt.Sprintf("%s/", project.CCompiler))

	return buildDirectory.String()
}

func AddDefaultConfigureOptions(options *strings.Builder, codeDirectory string, buildDirectory string, cCompiler string, linker string, buildMode string, defaultFreeStanding bool, testBuild bool) {
	var isFreeStanding = defaultFreeStanding && !testBuild

	argument.AddArgument(options, fmt.Sprintf("-S %s", codeDirectory))
	argument.AddArgument(options, fmt.Sprintf("-B %s", buildDirectory))
	argument.AddArgument(options, fmt.Sprintf("-D CMAKE_C_COMPILER=%s", cCompiler))
	argument.AddArgument(options, fmt.Sprintf("-D CMAKE_LINKER=%s", linker))
	argument.AddArgument(options, fmt.Sprintf("-D CMAKE_BUILD_TYPE=%s", buildMode))
	argument.AddArgument(options, fmt.Sprintf("-D FREESTANDING_BUILD=%t", isFreeStanding))
	argument.AddArgument(options, fmt.Sprintf("-D REPO_ROOT=%s", common.REPO_ROOT))
	argument.AddArgument(options, fmt.Sprintf("-D REPO_DEPENDENCIES=%s", common.REPO_DEPENDENCIES))
	argument.AddArgument(options, fmt.Sprintf("-D REPO_PROJECTS=%s", common.REPO_PROJECTS))
	argument.AddArgument(options, fmt.Sprintf("--graphviz=%s/output.dot", codeDirectory))

	var iwyuString = strings.Builder{}
	iwyuString.WriteString("-D CMAKE_C_INCLUDE_WHAT_YOU_USE=\"include-what-you-use;-w;-Xiwyu;")
	if isFreeStanding {
		iwyuString.WriteString("--no_default_mappings")
	}
	iwyuString.WriteString("\"")
	argument.AddArgument(options, iwyuString.String())

	argument.AddArgument(options, fmt.Sprintf("-D UNIT_TEST_BUILD=%t", testBuild))
}

func AddDefaultBuildOptions(options *strings.Builder, buildDirectory string, threads int, targets []string) {
	argument.AddArgument(options, fmt.Sprintf("--build %s", buildDirectory))
	argument.AddArgument(options, fmt.Sprintf("--parallel %d", threads))

	if len(targets) > 0 {
		targetsString := strings.Builder{}
		for _, target := range targets {
			targetsString.WriteString(target)
			targetsString.WriteString(" ")
		}
		argument.AddArgument(options, fmt.Sprintf("--target %s", targetsString.String()))
	}
}
