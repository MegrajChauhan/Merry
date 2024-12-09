# Style Guide

This style guide provides guidelines for writing and formatting code, commit messages, and branch names to ensure consistency and maintainability across the project.

## Table of Contents

- [General Guidelines](#general-guidelines)
- [Commit Message Conventions](#commit-message-conventions)
- [Branch Naming Conventions](#branch-naming-conventions)

## General Guidelines

- Write clear, concise, and self-explanatory code.
- Use meaningful variable and function names.
- Adhere to the language-specific best practices.
- Ensure your code is modular and reusable.
- Avoid code duplication.

## Commit Message Conventions

Commit messages should be clear and descriptive. Follow these conventions for commit messages:

- Use the past tense ("Added feature").
- Emphasize on the task ("Bug Fixed").
- Keep the subject line (first line) under 50 characters.
- Separate subject from body with a blank line.
- Provide a detailed description of the changes in the body.
- Include references to relevant issues or pull requests.

### Examples:

```
feature: Added new operation

Added a new operation that can be used to abstract some of the built-in functions.
```

```
fixed: Resolve crash on some inputs

Fixed an issue causing the library to crash with some input.
Closes #42.
```

## Branch Naming Conventions

Branch names should be descriptive and follow a consistent pattern. Use the following conventions for naming branches:

- Use lowercase letters and hyphens (`-`) to separate words.
- Include a prefix to indicate the type of branch:
  - `feature/` for new features
  - `fixes/` for bug fixes
  - `chore/` for maintenance tasks
  - `docs/` for documentation updates

### Examples:

```
feature/added_operation
fixes/crash_on_use
chore/update-dependencies
docs/improve-readme
```

---

By following this style guide, you contribute to a consistent and maintainable codebase, making it easier for everyone to collaborate and improve the project. Thank you for your contributions!

**Referenced from [GNOME Nepal](https://github.com/GNOME-Nepal/website)**
