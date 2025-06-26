# Merry VM

Merry is an attempt at reimagining computation exploration- a result of 2 years of architectural refinement. It is a powerful virtual machine designed to support multiple computation models, custom memory systems, and privilege-based control — all inside a unified, extensible architecture.

---

## What is Merry?

**Merry VM** is an experimental, multi-model virtual machine — built from scratch to explore **how computation can be simulated across diverse architectures**.

Unlike traditional VMs that stick to one paradigm (e.g., stack-based or register-based), Merry allows multiple computation models to **co-exist and collaborate** within a single runtime.

Each model is represented as a **virtual core (vcore)**, and every vcore can:
- Run its own instruction set
- Access its own or shared memory
- Follow strict or relaxed privilege rules

Merry is not "yet another toy VM". It is an ongoing attempt to rethink what VMs *can* be — for learning, experimenting, and exploring systems programming in a deeply customizable way.

---

## Core Concepts (Implemented or Planned)

| Feature | Description |
|--------|-------------|
| ✅ Multi-architecture Support | Stack-based, Register-based, Hybrid — all via separate *vcores* |
| ✅ Unified File Format | Modular format that adapts to new core types |
| ⚙️ Multi-State Execution | Simulates context switching between different execution states |
| ⚙️ MRS (Multi-RAM System) | Supports **Public**, **Private**, and **Shared** RAM blocks |
| ⚙️ Privilege Model | Fine-grained access control per vcore |
| ⚙️ Wild Request Handling | Interrupt-like system for async core signaling |
| 🚧 Subsystems | User-defined architectures running alongside the core VM |

---

##  Status

> ❗ **This project is in early preview.**  
> Testing is limited. Windows support is untested. No standard tooling or GUI yet.

This project at its current stage is for those interested in:
- Low-level VM design
- Systems simulation
- Watching a weird and powerful idea evolve

---

##  Preview Build

Check out the [latest preview release here](https://github.com/MegrajChauhan/Merry/releases).

Please take the warning seriously.
---

##  Dev Logs

> Development is tracked publicly in [`/devlog`](./devlog).

Follow along as I document weekly progress, decisions, bugs, breakdowns, breakthroughs, and what I’m building next.

---

##  Discussions & Feedback

Want to ask questions or offer feedback?  
Check out the [GitHub Discussions](https://github.com/MegrajChauhan/Merry/discussions) page — I’d love to hear your thoughts or ideas.

---

##  Support This Project

I'm a full-time student in Nepal working on Merry alone, during nights and weekends. If you believe in the vision of accessible, open, and deep systems experimentation — your support keeps this alive.

| Platform | Link |
|---------|------|
| 🧾 OpenCollective | *[pending approval]* |
| ❤️ Patreon | [https://patreon.com/YOURNAME](https://patreon.com/AryanChauhan) |
| 💡 Email (for Payoneer) | `your@email.com` |

Even a small tip or share means a lot. Thank you!

---

## Roadmap

- [ ] Implement Multi-RAM System (MRS)
- [ ] Add privilege enforcement per vcore
- [ ] Build testing framework
- [ ] Create user-defined sub-architecture interface
- [ ] Create example programs
- [ ] Improve documentation

---

##  Who Made This

**Merry** is built by one person — me.  
I’m a student, developer, and systems enthusiast trying to create something that lets people play with computing from the ground up.

Want to help or collaborate? Reach out via [GitHub Issues](https://github.com/MegrajChauhan/Merry/issues) or [email](mailto: megrajchauhan123@gmail.com).

---

## Give It a Star

If you found this project interesting, helpful, or just plain weird — leave a ⭐ on the repo to show support!

---

##  License

Merry is released under the **Merry Protective Source License (MPSL-1.0)**.  
This means it is open for study, improvement, and non-commercial sharing — but protected against unauthorized commercial use. See [LICENSE](./LICENSE) for full terms.