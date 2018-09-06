# Contributing to GLOVE

First of all, thanks for taking the time to contribute!

## Code Of Conduct

This project and everyone participating in it is governed by the [GLOVE Code of Conduct](CODE_OF_CONDUCT.md). By participating, you are expected to uphold this code. Please report unacceptable behavior to info@think-silicon.com.

## Reporting bugs / Requesting for a missing feature

GLOVE is considered as work in progress, therefore some GL ES 2.0 features are still missing and bugs can emerge as well. By raising a bug report or requesting for a missing feature you are contributing to the development of GLOVE and this is the first step of participating in the community.

Bug reports or missing features are filed in Github Issue tracker. If you are a Github beginner you can have a look in [Github Issues HowTo](https://help.github.com/articles/creating-an-issue/) so as to create a new issue.

GLOVE development team has already created a continuous integration framework used to check conformance with GL ES 2.0 spec, compatibility to various platforms, performance and stability. However, it's impossible to include all of testing or real life scenarios, therefore, reporting a bug found in your testing environment is more than welcome.

Moreover, requests for new features could help GLOVE development team to adjust GLOVE roadmap according to different use cases and various priorities.

When raising an issue, please keep in mind the set of simple rules mentioned below so that your issue can really help GLOVE development progress.

### General Issue Guidelines

* **Search existing issues:** Before raising an issue, search if it has been raised or if it's already in progress. Please have a look in [GLOVE Issues tracker](https://github.com/Think-Silicon/GLOVE/issues) and [GLOVE Known Issues](Docs/KnownIssues.md).
* **Report each issue separately:** Please don't collect multiple, unrelated problems into one issue.
* **Short description:** Long explanations usually harden the work of developers. Bullets or screenshots, where possible, make our lives easier.

### Bug Report Guidelines

* **Clear title and description** Please add a clear and descriptive title of the bug as well as a short description which will help GLOVE development team to understand the problem.
* **Steps to reproduce:** Please include a list of steps needed to reproduce the bug. The sooner we see the issue in our environment the sooner we can solve it.
* **Version number:** Please add the github version number in your report, as it might be solved in later versions.

## Contributing Code
### Contribution License Agreement

Contributing code to GLOVE project is more than welcome! However, before accepting any code change, we have to deal with some necessary legal stuff.

When contributing code to GLOVE, you have to fill out the [GLOVE Contributor License Agreement](https://think-silicon.com/terms_and_policies/thinksilicon-individual-contributor-license-agreement/). To make your life easier, this procedure is automated by [cla-assistant](https://github.com/cla-assistant/cla-assistant), when submitting a new Pull Request.

Please follow the instructions prompted by cla-bot to sign the CLA. Once you have finished with these legal stuff, we'll be able to accept your pull requests.

### Issue a Pull Request

For any code change/ improvement to GLOVE, please submit a pull request! For Github begginers, there's a [GitHub Pull Request HowTo](https://help.github.com/articles/about-pull-requests/).

To create the Pull Request, please follow the below steps:

1. Fork the repo and create your branch from master.
2. Make sure that your code is consistent with [GLOVE Coding style](#coding-style).
3. Make sure that your code compiles with --werror option ON and links correctly with the underlaying Vulkan driver.
4. Make sure that GLOVE demos run successfully.
5. Create and submit the Pull Request!

GLOVE team members will be assigned to review your pull requests. Once the pull requests are approved and pass our internal continuous integration checks, we will merge the pull requests. 

## Coding Style

All C++ code commited to GLOVE must adhere to [GLOVE Coding Style guide](CodingStyle.md). 

