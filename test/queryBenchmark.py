import requests
import time
import json

keyword_query_list = [
    # Longer example
    "deep learning for natural language processing",
    "transformer models in machine translation",
    "neural networks for image recognition",
    "convolutional neural networks for computer vision",
    "reinforcement learning applications in robotics",
    "unsupervised learning techniques",
    "generative adversarial networks for data augmentation",
    "natural language processing with BERT",
    "machine learning algorithms for predictive analytics",
    "explainable AI and interpretability in machine learning",
    "support vector machines for classification",
    "transfer learning in deep learning",
    "semi-supervised learning methods",
    "graph neural networks for social network analysis",
    "meta-learning for few-shot learning",
    # Short Example
    "attention all you need",
    "deep learning natural language processing",
    "transformer models machine translation",
    "neural networks image recognition",
    "convolutional neural networks computer vision",
    "reinforcement learning applications robotics",
    "unsupervised learning techniques",
    "generative adversarial networks data augmentation",
    "natural language processing BERT",
    "machine learning algorithms predictive analytics",
    "explainable AI interpretability machine learning",
    "support vector machines classification",
    "transfer learning deep learning",
    "semi-supervised learning methods",
    "graph neural networks social network analysis",
    "meta-learning few-shot learning",
    "sequence-to-sequence models text generation",
    "self-supervised learning neural networks",
    "attention mechanisms neural networks",
    "recurrent neural networks time series forecasting",
    "neural architecture search automated machine learning",
    "adversarial examples machine learning security",
    "big data analytics machine learning",
    "natural language understanding deep learning",
    "deep reinforcement learning game playing",
    "multi-task learning machine learning",
    "bioinformatics machine learning",
    "data mining machine learning",
    "computer vision object detection",
    "speech recognition deep learning",
    "robotics autonomous systems",
    "AI ethics machine learning",
    "machine learning healthcare",
    "recommender systems collaborative filtering",
    "image segmentation deep learning",
    "quantum computing machine learning",
    "bayesian networks probabilistic models",
    "deep generative models AI",
    "few-shot learning techniques",
    "causal inference machine learning",
    "human-computer interaction AI",
    "anomaly detection machine learning",
    "AI for social good",
    "neural network optimization techniques",
    "distributed machine learning systems",
    "machine learning model deployment",
]


def main():
    total_time = 0

    for index, query in enumerate(keyword_query_list):
        response = requests.post("http://localhost:5000/query", json={"query": query})
        elapsed_time = response.elapsed.microseconds / 1000.0
        total_time += elapsed_time

        print(f"Query#{index}\t{query}\t{elapsed_time} ms")

    average_time = total_time / len(keyword_query_list)
    print("\n---------------")
    print(f"Average query time: {average_time} ms")


if __name__ == "__main__":
    main()
