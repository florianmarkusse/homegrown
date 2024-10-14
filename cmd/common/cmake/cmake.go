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
	Folder              string
	CodeFolder          string
	DefaultFreeStanding bool
}

// If you add a project add it here
const KERNEL = "kernel"
const INTEROPERATION = "interoperation"
const UEFI_IMAGE_CREATOR = "uefi-image-creator"
const UEFI = "uefi"
const IMAGE_BUILDER = "image-builder"
const SHARED = "shared"

// and here
var kernelFolder = common.PROJECT_FOLDER + KERNEL + "/"
var interoperationFolder = common.PROJECT_FOLDER + INTEROPERATION + "/"
var uefiImageCreatorFolder = common.PROJECT_FOLDER + UEFI_IMAGE_CREATOR + "/"
var uefiFolder = common.PROJECT_FOLDER + UEFI + "/"
var imageBuilderFolder = common.PROJECT_FOLDER + IMAGE_BUILDER + "/"
var sharedFolder = common.PROJECT_FOLDER + SHARED + "/"

// and here
var PROJECT_STRUCTURES = map[string]*ProjectStructure{
	KERNEL: &ProjectStructure{
		Folder:              kernelFolder,
		CodeFolder:          kernelFolder + "code",
		DefaultFreeStanding: true,
	},
	INTEROPERATION: &ProjectStructure{
		Folder:              interoperationFolder,
		CodeFolder:          interoperationFolder + "code",
		DefaultFreeStanding: true,
	},
	UEFI_IMAGE_CREATOR: &ProjectStructure{
		Folder:              uefiImageCreatorFolder,
		CodeFolder:          uefiImageCreatorFolder + "code",
		DefaultFreeStanding: false,
	},
	UEFI: &ProjectStructure{
		Folder:              uefiFolder,
		CodeFolder:          uefiFolder + "code",
		DefaultFreeStanding: true,
	},
	IMAGE_BUILDER: &ProjectStructure{
		Folder:              imageBuilderFolder,
		CodeFolder:          imageBuilderFolder + "code",
		DefaultFreeStanding: false,
	},
	SHARED: &ProjectStructure{
		Folder:              sharedFolder,
		CodeFolder:          sharedFolder + "code",
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

func BuildDirectoryRoot(codeDirectory string, testBuild bool, cCompiler string) string {
	buildDirectory := strings.Builder{}
	buildDirectory.WriteString(fmt.Sprintf("%s/", codeDirectory))
	buildDirectory.WriteString("build/")
	if testBuild {
		buildDirectory.WriteString("test/")
	} else {
		buildDirectory.WriteString("prod/")
	}
	buildDirectory.WriteString(fmt.Sprintf("%s/", cCompiler))

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
