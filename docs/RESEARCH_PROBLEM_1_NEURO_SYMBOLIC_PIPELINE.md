# Research Report: 5-Stage Neuro-Symbolic & Reinforcement Learning Pipeline for Longitudinal Belief Revision

## 1. Problem Definition & Architectural Motivation

In archaeological science and doctoral research, modeling an individual scholar's evolving epistemic state over multi-year inquiry is the central challenge of intelligent assistance. A doctoral researcher reading hundreds of excavation reports, stratigraphy logs, and radiocarbon assays across 3 to 7 years constantly revises their beliefs.

Standard single-paradigm artificial intelligence fails on this task:
1. Generative LLMs alone suffer from catastrophic semantic drift, hallucinate causal links, and lose temporal track of premises established in earlier years.
2. Pure Neural Graph Networks (GNNs) lack logical provenance; they cannot provide the strict, verifiable justification chains required for scientific dissertation defense.
3. Pure Symbolic Solvers (Prolog, SAT solvers, Truth Maintenance Systems) collapse on real-world archaeological data because a single noisy outlier or taphonomic disturbance causes complete logical contradiction.
4. Standard Reinforcement Learning (Q-learning) cannot represent complex conjunctive hypotheses and overestimates unseen action values on sparse human interaction logs.

The 5-Stage Hybrid Pipeline solves this by decomposing Problem 1 into five mathematically optimal stages, aligning precisely with ArchaeoPhD's mission.

---

## 2. Alignment with ArchaeoPhD Core Principles

### 2.1 Archaeological Epistemology & Destructive Excavation
As established by Edward C. Harris (1979) and Martin Carver (1990), archaeological excavation physically destroys its own primary empirical record. The published literature and field archives are the only surviving surrogates. Therefore, beliefs must never be modeled as flat, immutable assertions of fact. Instead, they must be tracked as observer-dependent epistemic opinions subject to revision.

### 2.2 Taphonomic Contamination & Non-Binary Logic
Michael B. Schiffer (1987) demonstrated that archaeological strata undergo post-depositional cultural and natural transformations (bioturbation, leaching, redeposition). A mathematical belief revision system must tolerate sample contamination without arithmetic failure. The contamination parameter in the Noisy-AND formulation directly encodes taphonomic risk.

### 2.3 Scholarly Contradiction vs. Epistemic Ignorance
In traditional systems, scalar probabilities confuse an absence of evidence with intense academic controversy. If a hypothesis has zero evidence, standard systems may assign a default prior of 0.5. If another hypothesis has ten strong supporting radiocarbon dates and ten contradicting ceramic associations, its mean probability is also 0.5. Subjective Logic resolves this category error.

---

## 3. Mathematical Stage Breakdown & System Integration

### Stage 1: LLM + Calibrated NLI Cross-Encoder (Extraction)
* **Mathematical Formulation**:
  The neural cross-encoder produces calibrated relation probabilities via Platt temperature scaling:
  $$P_{\text{LLM}}(r \mid x_t, v_i) = \frac{\exp\left( z_r(x_t, v_i) / T_{\text{cal}} \right)}{\sum_{r' \in \{+, -, 0\}} \exp\left( z_{r'}(x_t, v_i) / T_{\text{cal}} \right)}$$
  Signed evidentiary weight injected into the graph:
  $$w_e(x_t \to v_i) = P_{\text{LLM}}(+ \mid x_t, v_i) - P_{\text{LLM}}(- \mid x_t, v_i) \in [-1, 1]$$
* **Integration in `experimental/`**:
  Directly augments `experimental/engine/analysis/llm_engine.hpp`. Rather than allowing raw LLM text generation to invent relationships, candidate edges are passed through an NLI Cross-Encoder to compute bounded, calibrated weights.

### Stage 2: Subjective Logic & Beta-Binomial Algebra (Node State)
* **Mathematical Formulation**:
  For accumulated positive evidence $r_v^+(t)$ and negative evidence $r_v^-(t)$ with uniform prior weight $W_0 = 2$:
  $$b_v(t) = \frac{r_v^+(t)}{r_v^+(t) + r_v^-(t) + W_0}, \quad d_v(t) = \frac{r_v^-(t)}{r_v^+(t) + r_v^-(t) + W_0}, \quad u_v(t) = \frac{W_0}{r_v^+(t) + r_v^-(t) + W_0}$$
  Epistemic Conflict Mass:
  $$\mathcal{C}_v(t) = 4 \cdot b_v(t) \cdot d_v(t) \cdot (1 - u_v(t)) \in [0, 1]$$
* **Integration in `experimental/`**:
  Extends `GraphNode` in `experimental/engine/analysis/graph_engine.hpp` and replaces heuristic confidence fields in `Claim` models. When $\mathcal{C}_v \to 1$, the node is flagged for contradiction auditing in `contradictions.hpp`.

### Stage 3: Directed Hypergraph Neural Network & Differentiable TMS
* **Mathematical Formulation**:
  A hyperedge $e = (S_e^+, S_e^-, v_e)$ represents a multi-premise scientific warrant. Premise satisfaction:
  $$\Phi_e^{(k)}(t) = \left( \prod_{u \in S_e^+} \big( (1 - \epsilon_u) b_u^{(k)}(t) + \epsilon_u \big) \right) \cdot \left( \prod_{z \in S_e^-} \big( 1 - (1 - \epsilon_z) b_z^{(k)}(t) \big) \right)$$
  Two-phase aggregation across historical warrants:
  $$r_v^{+(k+1)}(t) = r_{v,\text{direct}}^+(t) + \sum_{e \in \mathcal{I}^+(v)} \alpha_{e,v}(\mathbf{p}_t) \cdot w_e \cdot \Phi_e^{(k)}(t)$$
  $$r_v^{-(k+1)}(t) = r_{v,\text{direct}}^-(t) + \sum_{e \in \mathcal{I}^-(v)} \alpha_{e,v}(\mathbf{p}_t) \cdot |w_e| \cdot \Phi_e^{(k)}(t)$$
* **Integration in `experimental/`**:
  Replaces pairwise graph links with conjunctive hyperedges in `graph_engine.hpp`. Enables both **Downward Invalidation** (falsifying a foundation premise invalidates dependent conclusions) and **Upward Exoneration** (disproving a historical objection restores an abandoned hypothesis).

### Stage 4: Temporal Graph Attention & Neural Hawkes Process
* **Mathematical Formulation**:
  Relevance intensity of historical node $v$ given researcher perspective $\mathbf{p}_t$:
  $$\lambda_v(t) = \mu_v(\mathbf{p}_t) + \sum_{e_j : \tau_j < t} \Gamma_\theta(v, v_j) \cdot \exp(-\delta_v (t - \tau_j))$$
* **Integration in `experimental/`**:
  Provides dynamic temporal attention in `analytics_engine.hpp`. When a new discovery is made today, related dormant nodes from years ago experience an intensity spike, reviving forgotten insights.

### Stage 5: Conservative Q-Learning for Optimal Epistemic Interventions
* **Mathematical Formulation**:
  Dueling architecture $Q_\theta(\mathbf{s}_t, a_t) = V_\theta(\mathbf{s}_t) + (A_\theta(\mathbf{s}_t, a_t) - \frac{1}{|\mathcal{A}|}\sum A_\theta)$.
  Grounded Epistemic Reward:
  $$R(\mathbf{s}_t, a_t, \mathbf{s}_{t+1}) = \sum_{v \in \mathcal{V}} \lambda_v(t) (u_v(t) - u_v(t+1)) + \beta_1 \sum_{v \in \mathcal{V}} (\mathcal{C}_v(t) - \mathcal{C}_v(t+1)) + \beta_2 \mathbb{I}(\text{Exonerated } v) - \eta C_{\text{cog}}(a_t)$$
  CQL Bellman optimization penalizes out-of-distribution action values:
  $$\mathcal{L}_{\text{CQL}}(\theta) = \alpha_{\text{cql}} \, \mathbb{E}_{\mathbf{s}_t} \left[ \log \sum_{a} \exp Q_\theta(\mathbf{s}_t, a) - \mathbb{E}_{a \sim \hat{\pi}} [Q_\theta(\mathbf{s}_t, a)] \right] + \frac{1}{2} \mathcal{L}_{\text{TD}}(\theta)$$
* **Integration in `experimental/`**:
  Governs the notification policy in `thesis_audit.hpp`. Eliminates notification spam and sycophantic agreement by acting only when expected epistemic information gain exceeds cognitive interruption cost $C_{\text{cog}}$.

---

## 4. Implementation Strategy for the Experimental Workspace

1. **Step 1: C++ Subjective Logic & Hypergraph Engine (`engine/analysis/`)**:
   Implement `epistemic_logic.hpp` providing Subjective Logic tuples, Dirichlet updates, conflict mass calculations, and hyperedge premise propagation.
2. **Step 2: NLI Cross-Encoder Scoring Interface**:
   Add calibrated inference endpoints supporting ONNX/TorchScript weights for scientific sentence-pair entailment.
3. **Step 3: Hawkes Temporal Activation & Dueling Q-Policy**:
   Integrate time-stamped node activations and the CQL action selector for selective surfacing.
4. **Step 4: Evidence Graph Visual Synchronization (`dist/`)**:
   Update `EvidenceGraph` in the desktop interface to render epistemic uncertainty ($u_v$) as node transparency/border style and conflict mass ($\mathcal{C}_v$) as high-severity pulse indicators.
